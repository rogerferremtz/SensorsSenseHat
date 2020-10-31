# SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note

#Program client_smtp.c
#Version   1.3

#Author    Xorxe Oural Martínez & Roger Ferré Martínez
#Copyright (C) 2020

#License GNU/GPL, see COPYING
#This program is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#(at your option) any later version.
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#You should have received a copy of the GNU General Public License
#along with this program.  If not, see http://www.gnu.org/licenses/.



from sense_hat import SenseHat

import time

sense = SenseHat()

#Display X and R pattern
r = (255, 40, 0)	#Rosso Corsa for the letters
b = (0, 0, 0)		#Black for the background
w = (255, 255, 255)
y = (255, 255, 0)

pattern_pixels = [
	r, b, b, r, b, b, b, b,
	b, r, r, b, b, b, b, b,
	b, r, r, b, b, b, b, b,
	r, b, b, r, b, b, b, b,
	b, b, b, b, r, r, r, b,
	b, b, b, b, r, b, b, r,
	b, b, b, b, r, r, r, b,
	b, b, b, b, r, b, b, r
]


shaking_pixels = [
	y, y, y, y, y, y, y, y,
	y, y, y, y, y, y, y, y,
	y, b, b, y, y, b, b, y,
	y, b, b, y, y, b, b, y,
	y, y, y, y, y, y, y, y,
	y, y, y, b, b, y, y, y,
	y, y, b, y, y, b, y, y,
	y, b, y, y, y, y, b, y
]


while True:
	orientation = sense.get_orientation()
	acceleration = sense.get_accelerometer_raw()

	xa = round(acceleration['x'], 0)
	ya = round(acceleration['y'], 0)
	za = round(acceleration['z'], 0)

	x = round(orientation["pitch"], 0)
	y = round(orientation["roll"], 0)
	z = round(orientation["yaw"], 0)

	xx = abs(acceleration['x'])
	yy = abs(acceleration['y'])
	zz = abs(acceleration['z'])

	print("x = %s, y = %s, z = %s" % (x, y, z))

	time.sleep(0.5)

	if xx > 1 or yy > 1 or zz > 1:
                sense.set_pixels(shaking_pixels)
                time.sleep(2)
	else:
                sense.set_pixels(pattern_pixels)


	#Update the rotation of the display depending on which way the board is
	if xa == -1:
		sense.set_rotation(90)
	elif ya == 1:
		sense.set_rotation(0)
	elif ya == -1:
		sense.set_rotation(180)
	else:
		sense.set_rotation(270)

