/*
 *
 * BSD 2-Clause License
 *
 * Copyright (c) 2020, Till Mayer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package de.tlmrgvf.drtd;

import de.tlmrgvf.drtd.decoder.Decoder;
import de.tlmrgvf.drtd.decoder.HeadlessDecoder;
import de.tlmrgvf.drtd.dsp.Interpreter;
import de.tlmrgvf.drtd.dsp.component.BitConverter;
import de.tlmrgvf.drtd.gui.MainGui;
import de.tlmrgvf.drtd.thread.ProcessingThread;
import de.tlmrgvf.drtd.thread.SoundCardProcessingThread;
import de.tlmrgvf.drtd.thread.StdinProcessingThread;
import de.tlmrgvf.drtd.thread.UIUpdateThread;
import de.tlmrgvf.drtd.utils.LogHandler;
import de.tlmrgvf.drtd.utils.SettingsManager;
import de.tlmrgvf.drtd.utils.TargetLineWrapper;
import de.tlmrgvf.drtd.utils.Utils;

import javax.imageio.ImageIO;
import javax.sound.sampled.*;
import javax.swing.*;
import java.awt.*;
import java.io.IOException;
import java.io.InputStream;
import java.util.List;
import java.util.*;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.stream.Collectors;

public final class Drtd {
    private final static String RESOURCE_LOCATION = "res";
    public final static List<Image> ICONS = new LinkedList<>();
    private final static Options OPTIONS = new Options();
    private final static LogHandler HANDLER = new LogHandler();
    private final static Map<Class<?>, Interpreter> interpreters = new HashMap<>();
    private final static Mixer.Info[] AUDIO_MIXERS = AudioSystem.getMixerInfo();
    public final static String NAME = "drtd";
    private final static String[] ICON_NAMES = new String[]{"drtd.png", "drtd64.png", "drtd128.png"};

    private static Logger LOGGER;
    private static ProcessingThread processingThread;
    private static MainGui mainGui;
    private static TargetLineWrapper[] availableLines;
    private static int activeTargetLineIndex;
    private static UIUpdateThread updateThread;
    private static boolean guiMode;

    private Drtd() {
        assert false;
    }

    public static void main(String[] args) {
        processArgs(args); //Need to parse log level first
        guiMode = OPTIONS.headlessDecoder == null;
        if (OPTIONS.aaFont)
            System.setProperty("awt.useSystemAAFontSettings", "on");

        LOGGER = Drtd.getLogger(Drtd.class);
        LOGGER.info("Parsed options: " + OPTIONS);

        Thread.setDefaultUncaughtExceptionHandler((t, e) -> {
            LOGGER.severe("Uncaught exception in thread \"" + t.getName() + "\"!");
            System.err.println("An unexpected exception occurred and the application has to quit:");
            e.printStackTrace(System.err);
            System.exit(1);
        });

        SettingsManager.loadFile();
        SettingsManager.createFor(Drtd.class, false, true)
                .mapOption(Integer.class, () -> activeTargetLineIndex, i -> activeTargetLineIndex = i, 0)
                .loadAll();
        getAudioLines();

        if (OPTIONS.inputIndex == Options.INPUT_SHOW_AVAILABLE || OPTIONS.inputIndex >= availableLines.length) {
            System.out.println("Available audio inputs:");
            Arrays.stream(availableLines).map(w -> "\t" + w).forEach(System.out::println);
            System.exit(0);
            assert false;
        } else if (OPTIONS.inputIndex != Options.INPUT_NONE_SPECIFIED) {
            activeTargetLineIndex = OPTIONS.inputIndex;
        }
        LOGGER.fine("Selected input " + availableLines[activeTargetLineIndex]);

        if (guiMode) {
            LOGGER.info("Starting in UI mode");
            try {
                UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
            } catch (ClassNotFoundException |
                    InstantiationException |
                    IllegalAccessException |
                    UnsupportedLookAndFeelException e) {
                LOGGER.throwing("Drtd", "main", e);
                Utils.die();
            }

            try {
                for (String name : ICON_NAMES)
                    ICONS.add(ImageIO.read(Drtd.readResourceStream(name)));
            } catch (IOException e) {
                Drtd.getLogger(BitConverter.class).throwing("Drtd", "main", e);
                Utils.die();
            }

            for (ValueInterpreter i : ValueInterpreter.values())
                interpreters.put(i.getClassType(), i.getInstance());

            updateThread = new UIUpdateThread();
            updateThread.start();

            SwingUtilities.invokeLater(() ->
            {
                mainGui = new MainGui();

                mainGui.loadSettings();
                mainGui.setVisible(true);
            });
        } else {
            LOGGER.info("Starting in headless mode");
            Decoder<?> decoder = OPTIONS.headlessDecoder.getInstance();

            if (!(decoder instanceof HeadlessDecoder)) {
                LOGGER.severe("Selected decoder is not a headless decoder");
                Utils.die();
            }

            HeadlessDecoder<?, ?> headlessDecoder = (HeadlessDecoder<?, ?>) decoder;
            List<String> paramList = OPTIONS.decoderParameters;
            String[] availableParameters = headlessDecoder.getChangeableParameters();
            if (paramList.size() == availableParameters.length) {
                if (headlessDecoder.setupParameters(paramList.toArray(new String[0]))) {
                    decoder.setup();
                    startProcessing(decoder);
                    System.out.println("Ready.");
                } else {
                    showDecoderParameterErrorAndExit(headlessDecoder);
                }
            } else {
                showDecoderParameterErrorAndExit(headlessDecoder);
            }
        }
    }

    private static void showDecoderParameterErrorAndExit(HeadlessDecoder<?, ?> headlessDecoder) {
        String[] availableParams = headlessDecoder.getChangeableParameters();
        System.out.print("Invalid parameters! Available parameters: ");
        if (availableParams.length == 0) {
            System.out.println("None");
        } else {
            System.out.print("[");
            System.out.print(String.join("] [", availableParams));
            System.out.println("]");
        }
        printUsageAndExit();
    }

    public static boolean isGuiMode() {
        return guiMode;
    }

    public static void useDecoder(Decoder<?> decoder) {
        assert decoder != null;

        ProcessingThread old = Drtd.getProcessingThread();
        if (old != null) {
            old.getDecoder().saveSettings();
            old.getDecoder().onTeardown();
        }

        LOGGER.fine("Set new decoder " + decoder);
        stopProcessing();
        decoder.setup();
        Drtd.getUpdateThread().setDecoder(decoder);
        startProcessing(decoder);
    }

    private static void getAudioLines() {
        LOGGER.info("Reading available audio lines...");
        LinkedList<TargetLineWrapper> goodLines = new LinkedList<>();
        int lineIndex = 0;

        for (Mixer.Info mixerInfo : AUDIO_MIXERS) {
            final String description = String.format("%s (%s)", mixerInfo.getName(), mixerInfo.getDescription());
            LOGGER.info(String.format(
                    "\tMixer %s [%s, %s]",
                    description,
                    mixerInfo.getVendor(),
                    mixerInfo.getVersion())
            );
            final Mixer mixer = AudioSystem.getMixer(mixerInfo);

            for (Line.Info targetLine : mixer.getTargetLineInfo()) {
                LOGGER.info(String.format(
                        "\t\tTargetLine Info: %s (%s)",
                        targetLine.toString(),
                        targetLine.getLineClass())
                );

                Line line;
                try {
                    line = mixer.getLine(targetLine);
                    if (line instanceof TargetDataLine) {
                        LOGGER.info("\t\t\tTargetDataLine is ready!");
                        goodLines.add(new TargetLineWrapper(
                                (TargetDataLine) line,
                                String.format("%d: %s", lineIndex++, description))
                        );
                    } else {
                        LOGGER.warning(String.format("\t\t\t\"%s\" is not a TargetDataLine", line));
                    }
                } catch (LineUnavailableException e) {
                    LOGGER.warning("\t\t\tTargetLine is unavailable!");
                }
            }
        }

        availableLines = goodLines.toArray(new TargetLineWrapper[0]);
        LOGGER.info(String.format("Got %d usable data lines:", availableLines.length));
        goodLines.stream().map(w -> "\t" + w).forEach(LOGGER::info);

        if (availableLines.length == 0) {
            LOGGER.severe("There are no available data lines!");
            Utils.die();
        } else if (activeTargetLineIndex >= availableLines.length) {
            LOGGER.warning("Invalid index, resetting");
            activeTargetLineIndex = 0;
        }
    }

    public static UIUpdateThread getUpdateThread() {
        return updateThread;
    }

    public static TargetLineWrapper[] getAvailableLines() {
        return availableLines;
    }

    public static void setActiveTargetLineIndex(int activeTargetLineIndex) {
        Drtd.activeTargetLineIndex = activeTargetLineIndex;

        Decoder<?> decoder = processingThread.getDecoder();
        stopProcessing();
        startProcessing(decoder);
    }

    public static int getActiveTargetLineIndex() {
        return activeTargetLineIndex;
    }

    public static Interpreter getInterpreter(Class<?> c) {
        Interpreter interpreter = interpreters.get(c);
        if (interpreter == null) {
            LOGGER.severe(String.format("No interpreter for \"%s\" registered!", c));
            Utils.die();
        }

        return interpreter;
    }

    public static ProcessingThread getProcessingThread() {
        return processingThread;
    }

    private static void stopProcessing() {
        if (processingThread != null) {
            processingThread.requestStop();
            try {
                processingThread.join();
            } catch (InterruptedException e) {
                LOGGER.throwing("Drtd", "stopProcessing", e);
                Utils.die();
            }
            /* Now the processing thread should be dead */
            processingThread.cleanUp();
        }
    }

    private static void startProcessing(Decoder<?> decoder) {
        assert processingThread == null || !processingThread.isAlive();

        processingThread = OPTIONS.inputSource.provider.createNewInstance(decoder);
        processingThread.setDaemon(guiMode);
        processingThread.start();
    }

    public static Logger getLogger(Class<?> c) {
        Logger ret = Logger.getLogger(c.getSimpleName());
        ret.setUseParentHandlers(false);
        ret.addHandler(HANDLER);
        ret.setLevel(OPTIONS.logLevel);
        return ret;
    }

    private static void processArgs(String[] args) {
        Iterator<String> iterator = Arrays.asList(args).iterator();
        boolean collectDecoderParameters = false;

        while (iterator.hasNext()) {
            String arg = iterator.next();

            if (collectDecoderParameters) {
                OPTIONS.decoderParameters.add(arg);
            } else {
                switch (arg) {
                    case "--no-text-aa":
                        OPTIONS.aaFont = false;
                        break;
                    case "-i":
                    case "--input":
                        if (iterator.hasNext()) {
                            try {
                                int index = Integer.parseInt(iterator.next());
                                if (index < 0)
                                    printUsageAndExit();

                                OPTIONS.inputIndex = index;
                            } catch (NumberFormatException e) {
                                OPTIONS.inputIndex = Options.INPUT_SHOW_AVAILABLE;
                            }
                        } else {
                            printUsageAndExit();
                        }
                        break;
                    case "-l":
                    case "--level":
                        if (iterator.hasNext()) {
                            try {
                                OPTIONS.logLevel = Level.parse(iterator.next());
                                if (OPTIONS.logLevel == null) printUsageAndExit();
                                HANDLER.setLevel(OPTIONS.logLevel);
                            } catch (IllegalArgumentException e) {
                                printUsageAndExit();
                            }
                        } else {
                            printUsageAndExit();
                        }
                        break;
                    case "-g":
                    case "--headless":
                        if (iterator.hasNext()) {
                            DecoderImplementation decoder = DecoderImplementation.findByName(iterator.next());
                            if (decoder == null) {
                                System.out.println("Unknown decoder!");
                                showAvailableDecoders();
                                printUsageAndExit();
                            } else if (!decoder.hasHeadlessAvailable()) {
                                System.out.println("The specified decoder can not be run in headless mode!");
                                showAvailableDecoders();
                                printUsageAndExit();
                            }

                            OPTIONS.headlessDecoder = decoder;
                        } else {
                            showAvailableDecoders();
                            printUsageAndExit();
                        }
                        break;
                    case "-h":
                    case "--help":
                        printUsageAndExit();
                        break;
                    case "-s":
                    case "--stdin":
                        if (iterator.hasNext()) {
                            OPTIONS.inputSource = InputSource.STDIN;
                            try {
                                int rate = Integer.parseInt(iterator.next());
                                if (rate < 0)
                                    printUsageAndExit();

                                OPTIONS.inputSampleRate = rate;
                            } catch (NumberFormatException e) {
                                System.out.println("A valid sample rate needs to be specified!");
                                printUsageAndExit();
                            }
                        } else {
                            System.out.println("A valid sample rate needs to be specified!");
                            printUsageAndExit();
                        }
                        break;
                    case "--bits":
                        if (iterator.hasNext()) {
                            try {
                                StdinProcessingThread.SampleSize sampleSize =
                                        StdinProcessingThread.SampleSize.getByBits(Integer.parseInt(iterator.next()));

                                if (sampleSize == null) {
                                    System.out.println("A valid amount of bits needs to be specified!");
                                    printUsageAndExit();
                                }

                                OPTIONS.inputSampleSize = sampleSize;
                            } catch (NumberFormatException e) {
                                System.out.println("A valid amount of bits needs to be specified!");
                                printUsageAndExit();
                            }
                        } else {
                            System.out.println("A valid amount of bits needs to be specified!");
                            printUsageAndExit();
                        }
                        break;
                    case "--big-endian":
                        OPTIONS.inputBigEndian = true;
                        break;
                    default:
                        if (OPTIONS.headlessDecoder == null) {
                            System.out.println("Decoder parameters can only be specified in headless mode!");
                            printUsageAndExit();
                        } else {
                            collectDecoderParameters = true;
                            OPTIONS.decoderParameters.add(arg);
                        }
                }
            }
        }
    }

    private static void showAvailableDecoders() {
        System.out.print("Available decoders: ");
        System.out.println(Arrays.stream(DecoderImplementation.values())
                .filter(DecoderImplementation::hasHeadlessAvailable)
                .map(Enum::name)
                .collect(Collectors.joining(", ")));
    }

    public static InputStream readResourceStream(String location) {
        return Drtd.class.getResourceAsStream(RESOURCE_LOCATION + "/" + location);
    }

    private static void printUsageAndExit() {
        System.out.println("Usage: java -jar " + NAME.toLowerCase() + ".jar [Options] [Decoder parameters]");
        System.out.println("Options:");
        System.out.println("    -g, --headless <Decoder>        Headless mode using the specified decoder. " +
                "Specify none to show available decoders.\n" +
                "                                    Leave parameters empty to show available arguments.");
        System.out.println("    -i, --input <Device index>      Use specific audio input device. " +
                "Specify \"-\" to show all available");
        System.out.println("    -s, --stdin <Sample rate>       Read samples directly from stdin sampled using" +
                " specified sample rate");
        System.out.println("        --bits <8/16>               When reading from stdin: sample size (default 8)");
        System.out.println("        --big-endian                When reading from stdin: endianess of samples > 8 bit" +
                " is big");
        System.out.println("    -h, --help                      Show this help");
        System.out.println("        --no-text-aa                Don't force text anti-aliasing");
        System.out.println("    -l, --level <LEVEL>             Set the log level");
        System.out.println("                                    Levels: OFF, SEVERE, WARNING,\n" +
                "                                          INFO, CONFIG, FINE,\n" +
                "                                          FINER, FINEST, ALL");

        System.exit(0);
        assert false;
    }

    public static MainGui getMainGui() {
        return mainGui;
    }

    private enum InputSource {
        SOUND_CARD((decoder) ->
                new SoundCardProcessingThread(decoder, availableLines[activeTargetLineIndex].getDataLine())),
        STDIN(((decoder -> new StdinProcessingThread(decoder,
                OPTIONS.inputSampleSize,
                OPTIONS.inputSampleRate,
                OPTIONS.inputBigEndian))));

        private final ProcessingThreadProvider provider;

        InputSource(ProcessingThreadProvider provider) {
            this.provider = provider;
        }
    }

    private interface ProcessingThreadProvider {
        ProcessingThread createNewInstance(Decoder<?> decoder);
    }

    private static class Options {
        private final static int INPUT_NONE_SPECIFIED = -1;
        private final static int INPUT_SHOW_AVAILABLE = -2;
        private Level logLevel = Level.OFF;
        private boolean aaFont = true;
        private DecoderImplementation headlessDecoder;
        private final List<String> decoderParameters = new LinkedList<>();
        private int inputIndex = INPUT_NONE_SPECIFIED;
        private InputSource inputSource = InputSource.SOUND_CARD;
        private int inputSampleRate = 44100;
        private StdinProcessingThread.SampleSize inputSampleSize = StdinProcessingThread.SampleSize.BITS_8;
        private boolean inputBigEndian = false;

        @Override
        public String toString() {
            return "Options{" +
                    "logLevel=" + logLevel +
                    ", aaFont=" + aaFont +
                    ", headlessDecoder=" + headlessDecoder +
                    ", decoderParameters=" + decoderParameters +
                    ", inputIndex=" + inputIndex +
                    ", inputSource=" + inputSource +
                    ", inputSampleRate=" + inputSampleRate +
                    ", inputSampleSize=" + inputSampleSize +
                    ", inputBigEndian=" + inputBigEndian +
                    '}';
        }
    }
}
