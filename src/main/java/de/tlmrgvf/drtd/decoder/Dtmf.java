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

package de.tlmrgvf.drtd.decoder;

import de.tlmrgvf.drtd.dsp.PipelineComponent;
import de.tlmrgvf.drtd.dsp.component.GoertzelFilter;
import de.tlmrgvf.drtd.dsp.pipeline.PipelineBuilder;
import de.tlmrgvf.drtd.utils.Utils;

import javax.swing.*;
import java.awt.*;
import java.util.LinkedList;

public final class Dtmf extends Decoder<Integer> {
    private final static int SAMPLE_RATE = 5512;
    private final static int FILTER_TAPS = SAMPLE_RATE / 50; //About 50 Hz per bin
    private final static int NEEDED_SAMPLES_FOR_SYMBOL = (int) (SAMPLE_RATE * .05);//> 50 ms
    private final static int MINIMAL_PAUSE_SAMPLES = (int) (SAMPLE_RATE * .01);//> 10 ms
    private final static int[] COLUMN_FREQUENCIES = {1209, 1336, 1477, 1633};
    private final static int[] ROW_FREQUENCIES = {697, 770, 852, 941};
    private final static char[][] MAP = {
            {'1', '2', '3', 'A'},
            {'4', '5', '6', 'B'},
            {'7', '8', '9', 'C'},
            {'*', '0', '#', 'D'}
    };
    private final JTextArea outputTextArea;
    private char lastSymbol = '-';
    private char lastValid = '-';
    private int sampleCount = 0;
    private int lastInterruptionLength = 0;

    public Dtmf() {
        super(Integer.class, SAMPLE_RATE);
        outputTextArea = new JTextArea();
        Utils.setupSmartAutoscroll(outputTextArea);
    }

    private static Integer findMax(Float[] inputs) {
        assert inputs.length > 0;

        float max = inputs[0];
        int maxIndex = 0;

        for (int i = 1; i < inputs.length; ++i) {
            var value = inputs[i];

            if (value > max) {
                maxIndex = i;
                max = value;
            }
        }

        return max == 0 ? null : maxIndex;
    }

    private static PipelineComponent<Float, Integer> createFilterBank(int[] frequencies) {
        var filters = new LinkedList<PipelineComponent<Float, Float>>();
        for (int frequency : frequencies)
            filters.add(new GoertzelFilter(FILTER_TAPS, frequency));

        return PipelineBuilder.createEmpty(Float.class).split(Integer.class, Dtmf::findMax, filters).build();
    }

    @Override
    protected PipelineComponent<Float, Integer> buildPipeline() {
        return PipelineBuilder.createEmpty(Float.class).split(Integer.class,
                i -> i[0] + i[1] * 10,
                createFilterBank(COLUMN_FREQUENCIES),
                createFilterBank(ROW_FREQUENCIES))
                .build();
    }

    @Override
    public void addGuiComponents(JPanel parent) {
        parent.setLayout(new BorderLayout());
        outputTextArea.setEditable(false);
        outputTextArea.setFont(Utils.FONT);
        outputTextArea.setBackground(parent.getBackground());
        outputTextArea.setLineWrap(true);
        outputTextArea.setAutoscrolls(true);

        Utils.addClearContextMenu(outputTextArea, () -> outputTextArea.setText(""));

        var scroll = new JScrollPane(
                outputTextArea,
                JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
                JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED
        );

        scroll.setBorder(BorderFactory.createLoweredBevelBorder());
        parent.add(scroll);
    }

    @Override
    protected void onPipelineResult(Integer result) {
        final char received = MAP[result / 10][result % 10];

        if (received != lastSymbol) {
            if (sampleCount > NEEDED_SAMPLES_FOR_SYMBOL) {
                if (lastValid == lastSymbol && lastInterruptionLength < MINIMAL_PAUSE_SAMPLES)
                    return;

                lastValid = lastSymbol;
                Utils.doSmartAutoscroll(outputTextArea, Character.toString(lastSymbol));
                lastInterruptionLength = 0;
            } else {
                lastInterruptionLength += sampleCount;
            }

            lastSymbol = received;
            sampleCount = 0;
        } else {
            ++sampleCount;
        }
    }

    @Override
    protected void onSetup() {
    }
}
