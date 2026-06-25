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

Tiempo de paintGL(): 16.4222 ms | FPS teóricos: 60.8931  | Punots:  339800
Pre series:  0.002504
	Init buffers:  0.000351
	getVisible:  3.70522
	pload points:  0.003767
	Init buffers:  0.000401
	getVisible:  3.65861
	pload points:  0.002264
	Init buffers:  0.000351
	getVisible:  4.2282
	pload points:  0.002044

> [warming]
> Estaba compilando en debug, ahora ya es en release

Tiempo de paintGL(): 16.2552 ms | FPS teóricos: 61.5187  | Puntos:  1051000
Pre series:  0.002254
	Init buffers:  0.00027
	getVisible:  2.00384
	pload points:  0.004809
	Init buffers:  0.00028
	getVisible:  1.91799
	pload points:  0.003557
	Init buffers:  0.00027
	getVisible:  2.57712
	pload points:  0.003065

## VBO bucketing aritmethic out.

Tiempo de paintGL(): 17.14 ms | FPS teóricos: 58.3432  | Puntos:  1010900
Pre series:  0.002394
	Init buffers:  0.000261
	getVisible:  1.5867
	pload points:  0.00508
	Init buffers:  0.000251
	getVisible:  1.49566
	pload points:  0.004288
	Init buffers:  0.000281
	getVisible:  2.33487
	pload points:  0.004168
