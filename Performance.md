# Performance Table

## HASH:

- Al principio -> 16.4 ms (60 fps)
- 880 K Samples * 3 lines -> 83ms

## VBO con bucketing.

Tiempo de paintGL(): 16.4097 ms | FPS teóricos: 60.9396  | Punots:  334300
Pre series:  0.002594
	Init buffers:  0.000371
	getVisible:  4.01887
	pload points:  0.004719
	Init buffers:  0.000341
	getVisible:  3.89264
	pload points:  0.003306
	Init buffers:  0.00032
	getVisible:  5.29135
	pload points:  0.003146


## VBO bucking one pass:

Tiempo de paintGL(): 16.3727 ms | FPS teóricos: 61.0772  | Punots:  337700
Pre series:  0.002595
	Init buffers:  0.000371
	getVisible:  2.77729
	pload points:  0.004569
	Init buffers:  0.000361
	getVisible:  2.73202
	pload points:  0.003376
	Init buffers:  0.00035
	getVisible:  3.2835
	pload points:  0.003206
