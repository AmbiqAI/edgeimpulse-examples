# Concat and convert palette texture
run the following command to concat the indice file and palette file fot the L8 L4 L2 L1 color format:

```

cat xxxx_palette.16.rgba xxxx_indices.16.gray  > xxxx_indexed_xbits.bin

```

Note: put palette file in front of the indices file.



run the following comamnd to convert the bin file to header file if header file is used:

```

xxd -i -c 16  xxxx_indexed_xbits.bin xxxx_indexed_xbits.h

```