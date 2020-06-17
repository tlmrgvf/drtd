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

package de.tlmrgvf.drtd.utils.bch;

import java.util.ArrayList;
import java.util.Arrays;

public final class FFMatrix {
    private final static ArrayList<Integer[][]> PERMUTATION_CACHE = new ArrayList<>(Arrays.asList(new Integer[][]{{}}, new Integer[][]{{1}}, new Integer[][]{{1, 2}, {2, 1}}));

    private final int rows;
    private final int columns;
    private final Z2Polynomial[][] values; //rows x columns
    private final FiniteField field;

    public FFMatrix(FiniteField field, Z2Polynomial[][] values) {
        assert values != null;

        this.field = field;
        this.rows = values.length;
        this.columns = rows == 0 ? 0 : values[0].length;
        this.values = values;
    }

    public static FFMatrix identity(FiniteField field, int n) {
        Z2Polynomial[][] newValues = new Z2Polynomial[n][n];

        for (int r = 0; r < n; ++r) {
            for (int c = 0; c < n; ++c) {
                newValues[r][c] = new Z2Polynomial((c == r) ? 1 : 0);
            }
        }

        return new FFMatrix(field, newValues);
    }

    public static FFMatrix zero(FiniteField field, int n) {
        Z2Polynomial[][] newValues = new Z2Polynomial[n][n];

        for (int r = 0; r < n; ++r) {
            for (int c = 0; c < n; ++c) {
                newValues[r][c] = new Z2Polynomial();
            }
        }

        return new FFMatrix(field, newValues);
    }

    private static int faculty(int n) {
        int res = 1;

        for (int i = 2; i <= n; ++i) {
            res *= i;
        }

        return res;
    }

    private static Integer[][] permutations(int n) {
        assert n > 0;
        if (PERMUTATION_CACHE.size() > n && PERMUTATION_CACHE.get(n) != null)
            return PERMUTATION_CACHE.get(n);

        int permutations = faculty(n);
        int previousPermutations = faculty(n - 1);
        Integer[][] result = new Integer[permutations][n];
        Integer[][] partialPermutations = permutations(n - 1);

        for (int perm = 0; perm < permutations; ++perm) {
            int flip = perm / previousPermutations;
            Integer[] toPermutate = partialPermutations[perm % previousPermutations];
            Integer[] permutationResult = result[perm];

            System.arraycopy(toPermutate, 0, permutationResult, 0, toPermutate.length);

            if (flip == 0) {
                permutationResult[n - 1] = n;
            } else {
                permutationResult[n - 1] = permutationResult[flip - 1];
                permutationResult[flip - 1] = n;
            }
        }

        PERMUTATION_CACHE.add(result);
        return result;
    }

    public Z2Polynomial[][] getValues() {
        return values;
    }

    public int getColumns() {
        return columns;
    }

    public int getRows() {
        return rows;
    }

    public FFMatrix inverse() {
        assert rows == columns;
        return adjugate().multiply(field.rootInverse(determinant()));
    }

    public FFMatrix shrink() {
        assert rows > 0 && columns > 0 && rows == columns;
        if (rows == 1) {
            return new FFMatrix(field, new Z2Polynomial[][]{});
        }

        Z2Polynomial[][] newValues = new Z2Polynomial[rows - 1][columns - 1];
        for (int r = 0; r < rows - 1; ++r) {
            System.arraycopy(values[r], 0, newValues[r], 0, columns - 1);
        }

        return new FFMatrix(field, newValues);
    }

    private FFMatrix adjugate() {
        assert rows == columns;

        if (rows == 1) {
            Z2Polynomial entry = values[0][0];
            return new FFMatrix(field, new Z2Polynomial[][]{{new Z2Polynomial(entry.isZero() ? 0 : 1)}});
        }

        if (rows == 2) {
            return new FFMatrix(field, new Z2Polynomial[][]{{values[1][1], values[0][1]}, {values[1][0], values[0][0]}});
        }

        Z2Polynomial[][] newValues = new Z2Polynomial[rows][columns];
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < columns; ++c) {
                newValues[r][c] = minor(c, r).determinant(); //Flip r and c to transpose
            }
        }

        return new FFMatrix(field, newValues);
    }

    private FFMatrix minor(int rr, int cr) {
        Z2Polynomial[][] newValues = new Z2Polynomial[rows - 1][columns - 1];
        int newRow = 0;
        int newColumn = 0;

        for (int r = 0; r < rows; ++r) {
            if (rr == r)
                continue;

            for (int c = 0; c < columns; ++c) {
                if (cr != c)
                    newValues[newRow][newColumn++] = values[r][c];
            }

            newColumn = 0;
            ++newRow;
        }

        return new FFMatrix(field, newValues);
    }

    public FFMatrix add(FFMatrix matrix) {
        assert rows == matrix.rows && columns == matrix.columns;
        Z2Polynomial[][] newValues = new Z2Polynomial[rows][columns];

        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < columns; ++c) {
                newValues[r][c] = values[r][c].add(matrix.values[r][c]);
            }
        }

        return new FFMatrix(field, newValues);
    }

    public FFMatrix multiply(Z2Polynomial scalar) {
        Z2Polynomial[][] newValues = new Z2Polynomial[rows][columns];

        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < columns; ++c) {
                newValues[r][c] = field.multiplyRoots(values[r][c], scalar);
            }
        }

        return new FFMatrix(field, newValues);
    }

    public FFMatrix multiply(FFMatrix matrix) {
        assert columns == matrix.rows;
        Z2Polynomial[][] newValues = new Z2Polynomial[rows][matrix.columns];

        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < matrix.columns; ++c) {
                Z2Polynomial sum = new Z2Polynomial();

                for (int sc = 0; sc < columns; ++sc) {
                    sum = sum.add(field.multiplyRoots(values[r][sc], matrix.values[sc][c]));
                }

                newValues[r][c] = sum;
            }
        }

        return new FFMatrix(field, newValues);
    }

    public Z2Polynomial determinant() {
        assert rows == columns;

        if (rows == 1)
            return values[0][0];

        Z2Polynomial result = new Z2Polynomial();
        for (Integer[] map : permutations(rows)) {
            Z2Polynomial product = new Z2Polynomial(0b1);

            for (int i = 0; i < rows; ++i)
                product = field.multiplyRoots(values[i][map[i] - 1], product);

            result = result.add(product);
        }

        return result;
    }

    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder();

        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < columns; ++c) {
                builder.append(values[r][c]);
                builder.append(" ; ");
            }
            builder.append('\n');
        }

        return "FFMatrix{" +
                "rows=" + rows +
                ", columns=" + columns +
                ", entries=\n" + builder.toString() +
                '}';
    }
}
