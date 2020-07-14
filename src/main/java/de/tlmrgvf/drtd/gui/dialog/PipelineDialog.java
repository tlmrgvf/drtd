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

package de.tlmrgvf.drtd.gui.dialog;

import de.tlmrgvf.drtd.Drtd;
import de.tlmrgvf.drtd.decoder.Decoder;
import de.tlmrgvf.drtd.dsp.Interpreter;
import de.tlmrgvf.drtd.dsp.PipelineComponent;
import de.tlmrgvf.drtd.gui.utils.Canvas;
import de.tlmrgvf.drtd.gui.utils.Layer;
import de.tlmrgvf.drtd.utils.TargetLineWrapper;
import de.tlmrgvf.drtd.utils.Utils;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

public final class PipelineDialog extends JFrame {
    private final static int PADDING = 20;
    private final Canvas canvas;
    private final Layer draw;
    private final JLabel performanceLabel;

    private Decoder<?> decoder;
    private final JComboBox<TargetLineWrapper> inputCombobox;
    private final JComboBox<String> shownValue;

    public PipelineDialog() {
        setTitle("Pipeline");
        setLayout(new BorderLayout());
        setIconImages(Drtd.ICONS);
        setDefaultCloseOperation(JDialog.HIDE_ON_CLOSE);
        setMinimumSize(new Dimension(340, 200));
        canvas = new Canvas();
        draw = canvas.createLayer(0, 0, 10, 10);

        var centerPanel = new JPanel(new GridBagLayout());
        centerPanel.add(canvas);

        var listener = new ListenerImpl();
        canvas.addMouseListener(listener);
        JScrollPane scrollPane = new JScrollPane(
                centerPanel,
                JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
                JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED
        );

        canvas.setToolTipText("Right click: configure, Ctrl & left/right click:"
                + " Monitor input/output");

        var resetButton = new JButton("Reset Pipeline");
        resetButton.addActionListener((e) -> {
            if (JOptionPane.showConfirmDialog(
                    PipelineDialog.this,
                    "Are you sure you want to reset the Pipeline?\n"
                            + "This will undo all changes you made!", "Confirm reset",
                    JOptionPane.YES_NO_OPTION,
                    JOptionPane.WARNING_MESSAGE) == JOptionPane.YES_OPTION) {
                Drtd.getMainGui().updateDecoder();
            }
        });
        performanceLabel = new JLabel();

        var lowerPanel = new JPanel();
        lowerPanel.setLayout(new BoxLayout(lowerPanel, BoxLayout.X_AXIS));
        lowerPanel.setBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4));
        lowerPanel.add(resetButton);
        lowerPanel.add(Box.createHorizontalGlue());
        lowerPanel.add(performanceLabel);
        resetButton.setAlignmentX(Component.CENTER_ALIGNMENT);

        inputCombobox = new JComboBox<>(Drtd.getAvailableLines());
        inputCombobox.setSelectedIndex(Drtd.getActiveTargetLineIndex());
        inputCombobox.setFont(inputCombobox.getFont().deriveFont(10F));
        inputCombobox.setMaximumSize(new Dimension(200, Integer.MAX_VALUE));
        inputCombobox.addItemListener(listener);

        var upperPanel = new JPanel();
        upperPanel.setLayout(new BoxLayout(upperPanel, BoxLayout.Y_AXIS));

        shownValue = new JComboBox<>(new String[]{"(Nothing available)"});
        shownValue.setEnabled(false);
        shownValue.setEditable(false);
        shownValue.addItemListener(listener);

        Utils.addLabeledComponent(upperPanel, inputCombobox, "Input device");
        Utils.addLabeledComponent(upperPanel, shownValue, "Monitored value");

        scrollPane.setBorder(Utils.createLabeledBorder("Pipeline"));

        add(scrollPane, BorderLayout.CENTER);
        add(lowerPanel, BorderLayout.SOUTH);
        add(upperPanel, BorderLayout.NORTH);

        addComponentListener(listener);
        Utils.addEscapeHandler(scrollPane, this);
    }

    public void setInterpreter(Interpreter interpreter) {
        if (interpreter == null) {
            shownValue.setModel(new DefaultComboBoxModel<>(new String[]{"(Nothing available)"}));
            shownValue.setEnabled(false);
        } else {
            shownValue.setModel(new DefaultComboBoxModel<>(interpreter.getViewableValues()));
            shownValue.setEnabled(true);
        }

        shownValue.setSelectedIndex(0);
    }

    public void setDecoder(Decoder<?> decoder) {
        this.decoder = decoder;
        Dimension dims = Utils.resize(decoder.getPipeline().calculateSize(draw.getGraphics()),
                2 * PADDING,
                2 * PADDING);

        draw.setWidth(dims.width);
        draw.setHeight(dims.height);
        canvas.setMinimumSize(dims);
        canvas.setPreferredSize(dims);
        canvas.setSize(dims);
        canvas.resizeLayers();

        pack();
        redraw();
    }

    public void updatePerformance(float duration) {
        performanceLabel.setText(String.format("%.2f microseconds average", duration));
    }

    @Override
    public void setVisible(boolean b) {
        super.setVisible(b);
        setState(NORMAL);
    }

    public void redraw() {
        draw.clear();
        final var graphics = draw.getGraphics();
        graphics.translate(PADDING, PADDING);

        decoder.getPipeline().draw(new Point(), draw.getGraphics());
        canvas.drawLayers(true);
    }

    private enum ClickType {
        OPEN_DIALOG,
        MONITOR_INPUT,
        MONITOR_OUTPUT
    }

    private class ListenerImpl extends MouseAdapter implements ComponentListener, ActionListener, ItemListener {

        @Override
        public void componentShown(ComponentEvent componentEvent) {
            setLocationRelativeTo(Drtd.getMainGui());
        }

        @Override
        public void mouseClicked(MouseEvent mouseEvent) {
            int mods = mouseEvent.getModifiersEx();
            int but = mouseEvent.getButton();
            mouseEvent.translatePoint(-PADDING, -PADDING);

            boolean ctrl = (mods & MouseEvent.CTRL_DOWN_MASK) != 0;
            ClickType type;
            if (but == MouseEvent.BUTTON1 && ctrl) {
                type = ClickType.MONITOR_INPUT;
            } else if (but == MouseEvent.BUTTON3) {
                type = ctrl ? ClickType.MONITOR_OUTPUT : ClickType.OPEN_DIALOG;
            } else {
                return;
            }

            var clicked = decoder.getPipeline().getClickedComponent(
                    new Point(mouseEvent.getX(), mouseEvent.getY()),
                    draw.getGraphics());

            if (clicked != null) {
                if (clicked.getType() == PipelineComponent.ComponentType.PARALLEL_PIPELINE
                        && type != ClickType.MONITOR_OUTPUT) {
                    return;
                }

                if (type == ClickType.OPEN_DIALOG) {
                    clicked.showConfigureDialog();
                } else if (type == ClickType.MONITOR_INPUT || type == ClickType.MONITOR_OUTPUT) {
                    clicked.setMonitoring(clicked != decoder.getOutput()
                            && (type != ClickType.MONITOR_INPUT || clicked == decoder.getInput()));

                    var waterfall = Drtd.getMainGui().getWaterfall();
                    waterfall.setSampleRate(but == MouseEvent.BUTTON1
                            ? clicked.getInputSampleRate()
                            : clicked.getOutputSampleRate());
                    waterfall.enableMarker(decoder.getInput().isBeingMonitored());
                }
            }

            redraw();
        }

        @Override
        public void itemStateChanged(ItemEvent itemEvent) {
            if (itemEvent.getStateChange() == ItemEvent.SELECTED) {
                if (itemEvent.getSource() == inputCombobox) {
                    Drtd.setActiveTargetLineIndex(inputCombobox.getSelectedIndex());
                } else if (itemEvent.getSource() == shownValue) {
                    Drtd.getMainGui().getValueInterpreter().view(shownValue.getSelectedIndex());
                }
            }
        }

        @Override
        public void actionPerformed(ActionEvent actionEvent) {
        }

        @Override
        public void componentResized(ComponentEvent componentEvent) {

        }

        @Override
        public void componentMoved(ComponentEvent componentEvent) {

        }

        @Override
        public void componentHidden(ComponentEvent componentEvent) {

        }
    }
}
