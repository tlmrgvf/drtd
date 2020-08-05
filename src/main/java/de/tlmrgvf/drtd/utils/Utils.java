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

/*
 */
package de.tlmrgvf.drtd.utils;

import de.tlmrgvf.drtd.Drtd;

import javax.swing.*;
import javax.swing.border.Border;
import javax.swing.border.EtchedBorder;
import javax.swing.text.DefaultCaret;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.KeyEvent;
import java.io.File;
import java.net.URISyntaxException;
import java.util.Arrays;
import java.util.logging.Logger;

public final class Utils {
    private final static Logger LOGGER = Drtd.getLogger(Utils.class);
    private final static String[] ASCII_ESCAPE = {
            "<NUL>",
            "<SOH>",
            "<STX>",
            "<ETX>",
            "<EOT>",
            "<ENQ>",
            "<ACK>",
            "<BEL>",
            "<BS>",
            "<TAB>",
            "<LF>",
            "<VT>",
            "<FF>",
            "<CR>",
            "<SO>",
            "<SI>",
            "<DLE>",
            "<DC1>",
            "<DC2>",
            "<DC3>",
            "<DC4>",
            "<NAK>",
            "<SYN>",
            "<ETB>",
            "<CAN>",
            "<EM>",
            "<SUB>",
            "<ESC>",
            "<FS>",
            "<GS>",
            "<RS>",
            "<US>"
    };

    public static Font FONT;
    public static File ROOT_FOLDER;

    static {
        try {
            /*
             * https://stackoverflow.com/questions/4871051/how-to-get-the-current-working-directory-in-java
             * We want the config file to be where the .jar was run from
             */
            ROOT_FOLDER = new File(Drtd.class.getProtectionDomain().getCodeSource().getLocation().toURI());
            if (!ROOT_FOLDER.isDirectory()) ROOT_FOLDER = ROOT_FOLDER.getParentFile();
            if (ROOT_FOLDER == null) {
                LOGGER.severe("Could not get root folder!");
                die();
            }

            LOGGER.info("Root folder: " + ROOT_FOLDER.getAbsolutePath());
        } catch (URISyntaxException e) {
            LOGGER.throwing("Utils", "<clinit>", e);
            die();
        }

        FONT = Font.decode(Font.MONOSPACED).deriveFont(Font.BOLD, 12);
    }

    private Utils() {
        assert false;
    }

    public static float linearInterpolate(float floatIndex, float[] inputValues) {
        final float error = floatIndex - (int) floatIndex;
        return (error * inputValues[Math.min(inputValues.length - 1, (int) (floatIndex + 1))]) +
                ((1 - error) * inputValues[(int) floatIndex]);
    }

    public static boolean compareBit(boolean set, byte check, byte mask) {
        byte i = (byte) (check & mask);
        return (i == 0 && !set) || (i != 0 && set);
    }

    public static Border createLabeledBorder(String title) {
        return BorderFactory.createTitledBorder(BorderFactory.createEtchedBorder(EtchedBorder.RAISED), title);
    }

    public static void addLabeledComponent(JComponent parent, JComponent component, String title) {
        JPanel panel = new JPanel(new GridLayout(1, 1));
        panel.add(component);
        panel.setBorder(BorderFactory.createCompoundBorder(Utils.createLabeledBorder(title),
                BorderFactory.createEmptyBorder(5, 5, 5, 5))
        );
        parent.add(panel);
    }

    public static void die() {
        LOGGER.severe("A fatal error has occurred and the application has to quit.");
        Arrays.stream(Thread.currentThread().getStackTrace()).forEach(System.err::println);
        System.exit(1); //Should not return
        assert false;
    }

    public static void setupSmartAutoscroll(JTextArea area) {
        ((DefaultCaret) area.getCaret()).setUpdatePolicy(DefaultCaret.NEVER_UPDATE);
    }

    public static void doSmartAutoscroll(JTextArea area, String append) {
        Rectangle visibleRect = area.getVisibleRect();
        boolean scroll = (visibleRect.y + visibleRect.height) - area.getSize().height == 0;
        area.append(append);
        if (scroll)
            area.setCaretPosition(area.getText().length());
    }

    public static void ensureNotNull(Object o) {
        if (o == null)
            throw new IllegalArgumentException("Argument can't be null!");
    }

    public static void addEscapeHandler(JComponent component, JFrame toClose) {
        component.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW)
                .put(KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0), "actionEscapeClose");
        component.getActionMap().put("actionEscapeClose", new AbstractAction() {
            @Override
            public void actionPerformed(ActionEvent actionEvent) {
                toClose.setVisible(false);
            }
        });
    }

    public static int center(int total, int size) {
        return (total - size) / 2;
    }

    public static double calcDbAmplitude(double value, double ref) {
        return 20 * Math.log10(value / ref);
    }

    public static String escapeAscii(char character) {
        assert character <= 127;

        if (character > 31 && character < 127) {
            return String.valueOf(character);
        } else if (character == 127) {
            return "<DEL>";
        }

        return ASCII_ESCAPE[character];
    }

    public static double scaleLog(double value,
                                  double sourceMin,
                                  double sourceMax,
                                  double resultMin,
                                  double resultMax,
                                  boolean invert) {
        value = Utils.clampBetween(sourceMin, sourceMax, value);

        if (invert)
            value = sourceMax - value;

        sourceMax -= sourceMin;
        value -= sourceMin;
        value /= sourceMax;
        resultMax -= resultMin;

        double result = Math.pow(value + 1, 10) / 1024 * resultMax + resultMin;
        return invert ? resultMax - result : result;
    }

    public static void drawPath(Graphics g, int... path) {
        assert path.length % 2 == 0;
        Point last = null;
        for (int i = 0; i < path.length; i += 2) {
            Point p = new Point(path[i], path[i + 1]);
            if (last == null) {
                last = p;
                continue;
            }

            g.drawLine(last.x, last.y, p.x, p.y);
            last = p;
        }
    }

    public static void addClearContextMenu(JComponent component, Runnable action) {
        JPopupMenu contextMenu = new JPopupMenu();
        contextMenu.add("Clear").addActionListener((ActionEvent event) -> action.run());
        component.setComponentPopupMenu(contextMenu);
    }

    public static Dimension resize(Dimension dim, int width, int height) {
        return new Dimension((int) dim.getWidth() + width, (int) dim.getHeight() + height);
    }

    public static <T extends Comparable<T>> T clampBetween(T min, T max, T val) {
        if (val.compareTo(min) < 0) return min;
        if (val.compareTo(max) > 0) return max;
        return val;
    }
}
