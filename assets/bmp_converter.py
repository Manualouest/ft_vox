#!/bin/env python3

from PIL import Image
from sys import argv
from os import path

def i2b(n: int, length: int) -> bytes:
	return n.to_bytes(length, 'little')

if len(argv) == 1:
	print("Usage:", argv[0], "<fileN...>")
	exit(0)

def convert_img(in_path):
	out_path = path.splitext(in_path)[0] + ".bmp"
	try:
		in_img = Image.open(in_path).convert('RGB')
		in_img.save(out_path)

		out_file = open(out_path, "rb+")
		out_file.seek(10, 0)
		offset = int.from_bytes(out_file.read(4), 'little')
		out_file.seek(offset, 0)
		data_size = 3 * (in_img.width + in_img.width % 4) * in_img.height
		data = out_file.read(data_size)
		data += bytearray(data_size - len(data)) # no clue why we need this padding, probably because of height line padding

		dib_header = i2b(12, 4) + i2b(in_img.width, 2) + i2b(in_img.height, 2) + i2b(1, 2) + i2b(24, 2)
		out_len = 14 + 12 + len(data)
		bmp_header = b'BM' + i2b(out_len, 4) + i2b(0, 4) + i2b(14 + len(dib_header), 4)

		out_file.seek(0, 0)
		out_file.write(bmp_header)
		out_file.write(dib_header)
		out_file.write(data)
		out_file.truncate(out_len)
		out_file.close()
	except Exception as e:
		print("An error occurred while converting", in_path, ":")
		print(e)
	else:
		print("Converted", out_path, "!")

for f in argv[1:]:
	convert_img(f)