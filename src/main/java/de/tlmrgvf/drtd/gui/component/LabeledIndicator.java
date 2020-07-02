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

package de.tlmrgvf.drtd.gui.component;

import javax.swing.*;
import java.awt.*;

public final class LabeledIndicator extends JComponent {
    public final static Color YELLOW_OFF_COLOR = new Color(.4F, .3F, 0F);
    public final static Color YELLOW_ON_COLOR = new Color(.9F, .8F, 0F);
    public final static Color GREEN_OFF_COLOR = new Color(0F, .3F, 0F);
    public final static Color GREEN_ON_COLOR = new Color(0F, .8F, 0F);
    public final static Color RED_ON_COLOR = new Color(.85F, 0F, 0F);
    public final static Color RED_OFF_COLOR = new Color(.3F, 0F, 0F);

    public final static Dimension DIMENSIONS = new Dimension(25, 18);
    private final static int UPDATE_LIMIT = 30;

    private boolean on = false;
    private long lastUpdate;

    public LabeledIndicator(Color colorOn, Color colorOff, String label) {
        setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
        add(Box.createHorizontalStrut(2));
        add(new Indicator(colorOn, colorOff));
        add(Box.createHorizontalStrut(5));
        add(new JLabel(label));
        add(Box.createHorizontalStrut(2));
    }

    public void setState(boolean on) {
        if (this.on != on) {
            long time = System.currentTimeMillis();
            if (time - lastUpdate < (1 / (float) UPDATE_LIMIT) * 1000) return;
            lastUpdate = time;
            forceState(on);
        }
    }

    public void forceState(boolean on) {
        if (this.on != on) {
            this.on = on;
            repaint();
        }
    }

    public boolean isOn() {
        return on;
    }

    private final class Indicator extends JComponent {
        private final Color colorOff;
        private final Color colorOn;

        public Indicator(Color colorOn, Color colorOff) {
            this.colorOff = colorOff;
            this.colorOn = colorOn;

            setMaximumSize(DIMENSIONS);
            setMinimumSize(DIMENSIONS);
            setPreferredSize(DIMENSIONS);
            setBorder(BorderFactory.createLoweredBevelBorder());
        }

        @Override
        protected void paintComponent(Graphics g) {
            g.setColor(on ? colorOn : colorOff);
            g.fillRect(1, 1, getWidth() - 1, getHeight() - 1);
        }
    }

}
