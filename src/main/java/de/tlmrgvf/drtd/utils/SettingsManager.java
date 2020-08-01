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

package de.tlmrgvf.drtd.utils;

import de.tlmrgvf.drtd.Drtd;

import javax.swing.*;
import java.io.*;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.logging.Logger;

public final class SettingsManager {
    private final static Logger LOGGER = Drtd.getLogger(SettingsManager.class);
    private final static File FILE = new File(Utils.ROOT_FOLDER, ".drtd");
    private final static Map<Class<?>, SettingsManager> managers = new HashMap<>();
    private static Serialized settings;

    private final String className;
    private final boolean needsExplicitSave;
    private final boolean enableInHeadless;
    private final List<Option<?>> options = new LinkedList<>();
    private int index;

    private SettingsManager(Class<?> cls, boolean needsExplicitSave, boolean enableInHeadless) {
        this.className = cls.getName();
        this.needsExplicitSave = needsExplicitSave;
        this.enableInHeadless = enableInHeadless;
    }

    public static void saveManagers() {
        managers.values().stream().filter(m -> !m.needsExplicitSave).forEach(SettingsManager::saveAll);
    }

    public static SettingsManager createFor(Class<?> cls) {
        return createFor(cls, false, false);
    }

    public static SettingsManager createFor(Class<?> cls, boolean needsExplicitSave, boolean enableInHeadless) {
        if (managers.containsKey(cls))
            return managers.get(cls);

        SettingsManager manager = new SettingsManager(cls, needsExplicitSave, enableInHeadless);
        managers.put(cls, manager);
        return manager;
    }

    public void loadAll() {
        options.forEach(Option::load);
    }

    public void saveAll() {
        LOGGER.fine("Saving " + className);
        options.forEach(Option::save);
    }

    public static void loadFile() {
        if (FILE.exists()) {
            try (ObjectInputStream ois = new ObjectInputStream(new FileInputStream(FILE))) {
                Object obj = ois.readObject();
                if (obj instanceof Serialized)
                    settings = (Serialized) obj;
                else
                    throw new IllegalStateException("Invalid save file!");
            } catch (IOException | ClassNotFoundException e) {
                LOGGER.throwing("SettingsManager", "loadFile", e);
                Utils.die();
            }
        } else {
            settings = new Serialized(new HashMap<>());
        }
    }

    private static <T> T getSetting(String key, T initial, Class<T> saveClass) {
        if (settings.map.containsKey(key)) {
            Object saved = settings.map.get(key);
            if (saved == null || saveClass.isInstance(saved)) {
                return saveClass.cast(saved);
            } else {
                JOptionPane.showMessageDialog(null,
                        "Outdated save file!\nDelete the old file \"" + FILE.getAbsolutePath() +
                                "\"and restart the application!",
                        "Save file",
                        JOptionPane.ERROR_MESSAGE);
                Utils.die();
            }
        }

        return initial;
    }

    public <T extends Serializable> SettingsManager mapOption(Class<T> saveClass,
                                                              Provider<T> provider,
                                                              Setter<T> setter,
                                                              T initial) {
        options.add(new Option<>(saveClass,
                className + "." + index++,
                initial,
                setter,
                provider,
                enableInHeadless));
        return this;
    }

    public static void saveFile() {
        LOGGER.fine("Saving " + FILE);
        if (!FILE.exists()) {
            try {
                FILE.createNewFile();
            } catch (IOException e) {
                LOGGER.throwing("SettingsManager", "saveFile", e);
                Utils.die();
            }
        }

        try (ObjectOutputStream oos = new ObjectOutputStream(new FileOutputStream(FILE))) {
            oos.writeObject(settings);
        } catch (IOException e) {
            LOGGER.throwing("SettingsManager", "saveFile", e);
            Utils.die();
        }
    }

    private static class Option<T extends Serializable> {
        private final String key;
        private final T initialValue;
        private final Setter<T> setter;
        private final Provider<T> provider;
        private final Class<T> saveClass;
        private final boolean enableEvenWhenHeadless;

        public Option(Class<T> saveClass,
                      String key,
                      T initialValue,
                      Setter<T> setter,
                      Provider<T> provider,
                      boolean enableEvenWhenHeadless) {
            this.saveClass = saveClass;
            this.key = key;
            this.initialValue = initialValue;
            this.setter = setter;
            this.provider = provider;
            this.enableEvenWhenHeadless = enableEvenWhenHeadless;
        }

        void load() {
            if (Drtd.isGuiMode() || enableEvenWhenHeadless) {
                T value = SettingsManager.getSetting(key, initialValue, saveClass);
                setter.set(value);
                LOGGER.finer("Load " + key + " as " + value);
            }
        }

        void save() {
            if (Drtd.isGuiMode()) {
                T value = provider.get();
                SettingsManager.setSetting(key, value);
                LOGGER.finer("Save " + key + " as " + value);
            }
        }
    }

    private static <T extends Serializable> void setSetting(String key, T val) {
        settings.map.put(key, val);
    }

    private static class Serialized implements Serializable {
        private static final long serialVersionUID = 2130084025347175837L;
        private final Map<String, Object> map;

        public Serialized(Map<String, Object> map) {
            this.map = map;
        }
    }
}
