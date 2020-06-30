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

import de.tlmrgvf.drtd.decoder.ax25.protocol.Ax25Address;
import de.tlmrgvf.drtd.decoder.ax25.protocol.Ax25Packet;
import de.tlmrgvf.drtd.utils.Utils;

import javax.swing.*;
import javax.swing.table.DefaultTableModel;
import java.awt.*;
import java.util.Map;

public final class PacketDialog extends JDialog {

    public PacketDialog(Ax25Packet packet) {
        setLayout(new BorderLayout());
        setResizable(false);

        Ax25Address[] repeaters = packet.getRepeaters();
        Object[][] objArr = new Object[repeaters.length + 2][5];

        addAddress(objArr, packet.getSourceAddress(), 0);
        addAddress(objArr, packet.getDestinationAddress(), 1);

        for (int i = 0; i < repeaters.length; ++i) {
            addAddress(objArr, repeaters[i], i + 2);
        }

        JPanel propertyPanel = new JPanel();
        JTable addrTable = new JTable();
        JPanel rawPanel = new JPanel();
        addrTable.setModel(
                new DefaultTableModel(objArr, new String[]{"Address", "Type", "Repeated", "SSID", "Reserved"}) {
                    @Override
                    public boolean isCellEditable(int row, int column) {
                        return false;
                    }

                    @Override
                    public Class<?> getColumnClass(int columnIndex) {
                        return columnIndex == 2 ? Boolean.class : super.getColumnClass(columnIndex);
                    }
                });

        JScrollPane tableScroll = new JScrollPane(addrTable,
                JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
                JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
        tableScroll.setPreferredSize(new Dimension(400, 200));
        tableScroll.setBorder(Utils.createLabeledBorder("Address"));

        final Map<String, String> map = packet.getPropertyMap();
        propertyPanel.setBorder(Utils.createLabeledBorder("Properties"));
        propertyPanel.setLayout(new GridLayout(4 + map.size(), 2));
        addProperty(propertyPanel, "Type", packet.getType().getName());
        addProperty(propertyPanel, "Command", packet.getSourceAddress().isCommand() ? "Yes" : "No");
        addProperty(propertyPanel, "Control", String.format("0x%02x", packet.getControl()));
        addProperty(propertyPanel,
                packet.getSourceAddress().isCommand() ? "Poll" : "Final",
                packet.isPoll() ? "Yes" : "No");

        for (String desc : map.keySet())
            addProperty(propertyPanel, desc, map.get(desc));

        rawPanel.setBorder(Utils.createLabeledBorder("Raw data"));
        final Byte[] data = packet.getRaw();

        JTextArea hex = new JTextArea();
        hex.setEditable(false);
        hex.setBorder(BorderFactory.createLoweredBevelBorder());
        hex.setFont(Utils.FONT);
        hex.setBackground(getBackground());

        if (data == null) {
            hex.setText("[Packet has no data field]");
        } else {
            int lines = data.length / 16 + 1;
            hex.setRows(lines);
            hex.setColumns(71);
            hex.setLineWrap(true);

            final StringBuilder builder = new StringBuilder();
            for (int i = 0; i < lines; ++i) {
                builder.append(String.format("%03x ", i * 16));

                final StringBuilder ascii = new StringBuilder(16);
                for (int o = 0; o < 16; ++o) {
                    final int indx = (i * 16 + o);
                    if (indx < data.length) {
                        byte bte = data[indx];
                        builder.append(String.format("%02x ", bte));
                        ascii.append((bte < 32 || bte > 126) ? '.' : (char) bte);
                    } else {
                        builder.append("   ");
                    }
                }
                builder.append("  ");
                builder.append(ascii.toString());
            }

            hex.setText(builder.toString());
        }

        rawPanel.add(hex);

        add(propertyPanel, BorderLayout.NORTH);
        add(rawPanel, BorderLayout.CENTER);
        add(tableScroll, BorderLayout.SOUTH);
        pack();
    }

    private void addProperty(JPanel propertyPanel, String name, String value) {
        propertyPanel.add(new JLabel(name + ":"));
        propertyPanel.add(new JLabel(value));
    }

    private void addAddress(Object[][] arr, Ax25Address address, int offset) {
        arr[offset][0] = address.getName();
        arr[offset][1] = address.getType().getName();
        arr[offset][2] = address.isRepeated();
        arr[offset][3] = String.format("0x%02x", address.getSsid());
        arr[offset][4] = String.format("0x%02x", address.getReserved());
    }

}
