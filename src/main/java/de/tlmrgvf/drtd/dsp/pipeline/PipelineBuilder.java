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

package de.tlmrgvf.drtd.dsp.pipeline;

import de.tlmrgvf.drtd.dsp.PipelineComponent;
import de.tlmrgvf.drtd.utils.Utils;

import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;

/**
 * @param <T> Input type of the complete pipeline
 * @param <U> Intermediate output type
 */
public final class PipelineBuilder<T, U> {
    private final List<PipelineComponent<?, ?>> pipelineComponents;
    private final Class<U> resultClass;

    private PipelineBuilder(Class<U> inputClass) {
        pipelineComponents = new LinkedList<>();
        resultClass = inputClass;
    }

    private PipelineBuilder(PipelineComponent<T, U> component) {
        pipelineComponents = new LinkedList<>();
        pipelineComponents.add(component);
        resultClass = component.getResultClass();
    }

    private <V> PipelineBuilder(PipelineBuilder<T, V> builder, PipelineComponent<V, U> component) {
        this.pipelineComponents = builder.pipelineComponents;
        this.pipelineComponents.add(component);
        resultClass = component.getResultClass();
    }

    public static <T, U> PipelineBuilder<T, U> createForComponent(PipelineComponent<T, U> component) {
        Utils.ensureNotNull(component);
        return new PipelineBuilder<>(component);
    }

    public static <T> PipelineBuilder<T, T> createEmpty(Class<T> inputClass) {
        Utils.ensureNotNull(inputClass);
        return new PipelineBuilder<>(inputClass);
    }

    public <V> PipelineBuilder<T, V> then(PipelineComponent<U, V> next) {
        Utils.ensureNotNull(next);
        return new PipelineBuilder<>(this, next);
    }

    public <V> PipelineComponent<T, V> build(PipelineComponent<U, V> end) {
        Utils.ensureNotNull(end);
        pipelineComponents.add(end);
        return new Pipeline<>(end.getResultClass(), pipelineComponents.toArray(PipelineComponent[]::new));
    }

    public PipelineComponent<T, U> build() {
        if (pipelineComponents.isEmpty())
            throw new IllegalStateException("Pipeline is empty!");

        return new Pipeline<>(resultClass, pipelineComponents.toArray(PipelineComponent[]::new));
    }

    @SafeVarargs
    public final <V, X> PipelineBuilder<T, X> split(Class<X> resultClass,
                                                    MergeFunction<V, X> merger,
                                                    PipelineComponent<U, V>... pipelines) {
        return split(resultClass, merger, Arrays.asList(pipelines));
    }

    public final <V, X> PipelineBuilder<T, X> split(Class<X> resultClass,
                                                    MergeFunction<V, X> merger,
                                                    List<PipelineComponent<U, V>> pipelines) {
        if (pipelines.size() < 2)
            throw new IllegalArgumentException("Need at least two pipelines for a split!");
        Utils.ensureNotNull(resultClass);
        Utils.ensureNotNull(merger);

        return new PipelineBuilder<>(this, new ParallelPipeline<>(
                this.resultClass,
                resultClass,
                merger,
                pipelines));
    }
}
