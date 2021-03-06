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

package de.tlmrgvf.drtd.gui.utils.config;

import de.tlmrgvf.drtd.Drtd;

import javax.swing.*;
import java.awt.*;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public final class ConfigureDialog extends JDialog {
    private final static Map<Class<?>, ConfigureDialog> instances = new HashMap<>();
    private final Setting<?>[] settings;

    private ConfigureDialog(String title, Setting<?>... settings) {
        this.settings = settings;

        setTitle(title);
        setResizable(false);
        setIconImages(Drtd.ICONS);
        setDefaultCloseOperation(DISPOSE_ON_CLOSE);
        setMinimumSize(new Dimension(250, 0));

        GridLayout layout = new GridLayout(-1, 2);
        JPanel panel = new JPanel(layout);
        layout.setHgap(10);
        panel.setBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4));

        for (Setting<?> setting : settings) {
            panel.add(new JLabel(setting.name + ":"));
            panel.add(setting.createComponent());
            setting.loadComponentFromGetter();
        }

        add(panel);
        pack();
    }

    public static void showDialog(Class<?> configuring, String title, Setting<?>... settings) {
        if (instances.containsKey(configuring))
            instances.get(configuring).dispose();

        ConfigureDialog dialog = new ConfigureDialog(title, settings);
        dialog.setVisible(true);
        dialog.setLocationRelativeTo(Drtd.getMainGui().getPipelineDialog());
        instances.put(configuring, dialog);
    }

    public static void updateDialog(Class<?> configuring) {
        if (instances.containsKey(configuring))
            Arrays.stream(instances.get(configuring).settings).forEach(Setting::loadComponentFromGetter);
    }

    public static void closeAll() {
        instances.values().forEach(ConfigureDialog::dispose);
        instances.clear();
    }

    public interface Callback {
        void update(int id, double value);
    }
}
