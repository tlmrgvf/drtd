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
import de.tlmrgvf.drtd.dsp.Interpreter;
import de.tlmrgvf.drtd.dsp.component.BitConverter;
import de.tlmrgvf.drtd.gui.MainGui;
import de.tlmrgvf.drtd.utils.LogHandler;
import de.tlmrgvf.drtd.utils.SettingsManager;
import de.tlmrgvf.drtd.utils.TargetLineWrapper;
import de.tlmrgvf.drtd.utils.Utils;

import javax.imageio.ImageIO;
import javax.sound.sampled.*;
import javax.swing.*;
import java.awt.image.BufferedImage;
import java.io.IOException;
import java.io.InputStream;
import java.util.*;
import java.util.logging.Level;
import java.util.logging.Logger;

public final class Drtd {
    private final static String RESOURCE_LOCATION = "res";
    private final static Options OPTIONS = new Options();
    private final static LogHandler HANDLER = new LogHandler();
    private final static Map<Class<?>, Interpreter> interpreters = new HashMap<>();
    private final static Mixer.Info[] AUDIO_MIXERS = AudioSystem.getMixerInfo();
    public final static String NAME = "drtd";
    public static BufferedImage ICON;

    private static Logger LOGGER;
    private static ProcessingThread processingThread;
    private static MainGui mainGui;
    private static TargetLineWrapper[] availableLines;
    private static int activeTargetLineIndex;
    private static UIUpdateThread updateThread;

    private Drtd() {
        assert false;
    }

    public static void main(String[] args) {
        processArgs(args); //Need to parse log level first

        if (OPTIONS.aaFont)
            System.setProperty("awt.useSystemAAFontSettings", "on");

        LOGGER = Drtd.getLogger(Drtd.class);
        LOGGER.info("Parsed options: " + OPTIONS);

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
            ICON = ImageIO.read(Drtd.readResourceStream("drtd.png"));
        } catch (IOException e) {
            Drtd.getLogger(BitConverter.class).throwing("Drtd", "main", e);
            Utils.die();
        }

        Thread.setDefaultUncaughtExceptionHandler((t, e) -> {
            LOGGER.severe("Uncaught exception in thread \"" + t.getName() + "\"!");
            LOGGER.throwing("Drtd", "UncaughtHandler", e);
            System.exit(1);
        });

        SettingsManager.loadFile();
        SettingsManager.createFor(Drtd.class)
                .mapOption(Integer.class, () -> activeTargetLineIndex, i -> activeTargetLineIndex = i, 0)
                .loadAll();

        getAudioLines();

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
    }

    private static void getAudioLines() {
        LOGGER.info("Reading available audio lines...");
        var goodLines = new LinkedList<TargetLineWrapper>();
        int lineIndex = 0;

        for (var mixerInfo : AUDIO_MIXERS) {
            final var description = String.format("%s (%s)", mixerInfo.getName(), mixerInfo.getDescription());
            LOGGER.info(String.format(
                    "\tMixer %s [%s, %s]",
                    description,
                    mixerInfo.getVendor(),
                    mixerInfo.getVersion())
            );
            final var mixer = AudioSystem.getMixer(mixerInfo);
            final var targetLineInfos = mixer.getTargetLineInfo();

            for (var targetLine : targetLineInfos) {
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

        availableLines = goodLines.toArray(TargetLineWrapper[]::new);
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

        var decoder = processingThread.getDecoder();
        stopProcessing();
        startProcessing(decoder);
    }

    public static int getActiveTargetLineIndex() {
        return activeTargetLineIndex;
    }

    public static Interpreter getInterpreter(Class<?> c) {
        var interpreter = interpreters.get(c);
        if (interpreter == null) {
            LOGGER.severe(String.format("No interpreter for \"%s\" registered!", c));
            Utils.die();
        }

        return interpreter;
    }

    public static ProcessingThread getProcessingThread() {
        return processingThread;
    }

    public static void stopProcessing() {
        if (processingThread != null) {
            processingThread.requestStop();
            try {
                processingThread.join();
            } catch (InterruptedException e) {
                Utils.die();
            }
            //Now the processing thread should be dead
        }
    }

    public static void startProcessing(Decoder<?> decoder) {

        processingThread = new ProcessingThread(availableLines[activeTargetLineIndex].getDataLine(),
                decoder,
                decoder.getInputSampleRate());
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

        while (iterator.hasNext()) {
            String arg = iterator.next();
            switch (arg) {
                case "--no-text-aa":
                    OPTIONS.aaFont = false;
                    break;
                case "-l":
                case "--level":
                    if (iterator.hasNext()) {
                        OPTIONS.logLevel = Level.parse(iterator.next());
                        if (OPTIONS.logLevel == null) printUsageAndExit();
                        HANDLER.setLevel(OPTIONS.logLevel);
                    } else {
                        printUsageAndExit();
                    }
                    break;
                default:
                    printUsageAndExit();
            }
        }
    }

    public static InputStream readResourceStream(String location) {
        return Drtd.class.getResourceAsStream(RESOURCE_LOCATION + "/" + location);
    }

    private static void printUsageAndExit() {
        System.out.println("Usage: java -jar " + NAME.toLowerCase() + ".jar [OPTIONS]");
        System.out.println("Options:");
        System.out.println("    -h,--help               Show this help");
        System.out.println("       --no-text-aa         Don't force text anti-aliasing");
        System.out.println("    -l,--level <LEVEL>      Set the log level");
        System.out.println("        Levels: OFF, SEVERE, WARNING,\n" +
                "                INFO, CONFIG, FINE,\n" +
                "                FINER, FINEST, ALL");

        System.exit(0);
    }

    public static MainGui getMainGui() {
        return mainGui;
    }

    private static class Options {
        private Level logLevel = Level.OFF;
        private boolean aaFont = true;

        @Override
        public String toString() {
            return "Options{" +
                    "logLevel=" + logLevel +
                    ", aaFont=" + aaFont +
                    '}';
        }
    }
}
