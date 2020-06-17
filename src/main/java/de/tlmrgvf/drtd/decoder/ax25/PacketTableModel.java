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

package de.tlmrgvf.drtd.decoder.ax25;

import de.tlmrgvf.drtd.decoder.ax25.protocol.Ax25Packet;

import javax.swing.*;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.TableModel;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.util.LinkedList;
import java.util.List;

public final class PacketTableModel implements TableModel {
    private final static String[] COLUMNS = new String[]{
            "Type",
            "Raw message",
            "Packet information"
    };

    private final List<Object[]> packets = new LinkedList<>();
    private final List<TableModelListener> listeners = new LinkedList<>();
    private PacketDialog dialog;

    public PacketTableModel(JTable table) {
        table.setDefaultRenderer(JButton.class, (table1, value, isSelected, hasFocus, row, column) -> (JButton) value);
        table.setRowHeight(25);
        table.addMouseListener(new MouseAdapter() {
            @Override
            public void mousePressed(MouseEvent e) {
                int col = table.getColumnModel().getColumnIndexAtX(e.getX());
                int row = e.getY() / table.getRowHeight();
                if (row < getRowCount() && col < getColumnCount()) {
                    Object obj = getValueAt(row, col);
                    if (obj instanceof JButton) {
                        if (dialog != null) dialog.dispose();
                        dialog = new PacketDialog((Ax25Packet) packets.get(row)[3]);
                        dialog.setVisible(true);
                    }
                }
            }
        });
    }

    public void addPacket(Ax25Packet packet) {
        String message = "(No information field in packet)";

        if (packet.getData() != null) {
            final StringBuilder builder = new StringBuilder();
            for (byte b : packet.getData()) {
                if (b < 32 || b > 126) {
                    builder.append("<0x");
                    builder.append(String.format("%02X", b));
                    builder.append(">");
                } else {
                    builder.append((char) b);
                }
            }
            message = builder.toString();
        }

        packets.add(new Object[]{packet.getType(), message, new JButton("Show details"), packet});
        listeners.forEach(tml -> tml.tableChanged(
                new TableModelEvent(
                        this,
                        getRowCount() - 1,
                        getRowCount() - 1,
                        TableModelEvent.ALL_COLUMNS,
                        TableModelEvent.INSERT
                )
        ));
    }

    public void clear() {
        packets.clear();
        listeners.forEach(tml -> tml.tableChanged(new TableModelEvent(this)));
    }

    @Override
    public int getRowCount() {
        return packets.size();
    }

    @Override
    public int getColumnCount() {
        return COLUMNS.length;
    }

    @Override
    public Class<?> getColumnClass(int columnIndex) {
        switch (columnIndex) {
            case 0:
            case 1:
                return String.class;
            case 2:
                return JButton.class;
        }
        return null;
    }

    @Override
    public boolean isCellEditable(int rowIndex, int columnIndex) {
        return false;
    }

    @Override
    public String getColumnName(int column) {
        return COLUMNS[column];
    }

    @Override
    public Object getValueAt(int rowIndex, int columnIndex) {
        if (rowIndex >= packets.size()) return null;
        return packets.get(rowIndex)[columnIndex];
    }

    @Override
    public void setValueAt(Object aValue, int rowIndex, int columnIndex) {
    }

    @Override
    public void addTableModelListener(TableModelListener l) {
        listeners.add(l);
    }

    @Override
    public void removeTableModelListener(TableModelListener l) {
        listeners.remove(l);
    }
}
