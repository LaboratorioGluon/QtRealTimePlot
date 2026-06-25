# ToDo

## Mudar el escalado (Datos a pixeles) a la GPU mediante el shader.


## [ ] Usar VBO (Vertex Buffer Objects) estáticos o dinámicos:
En lugar de pasar verts.data() en la llamada setAttributeArray() (lo que obliga a OpenGL a copiar la memoria de la RAM a la VRAM en cada frame), debes almacenar los puntos directamente en la memoria de la tarjeta gráfica usando un VBO.

Como estás leyendo datos de la UART en tiempo real, puedes crear un búfer circular en la VRAM o actualizar el VBO solo cuando lleguen datos nuevos usando glBufferSubData(), y no en cada ciclo de paintGL().

## [] Implementar un algoritmo de Downsampling (Submuestreo) en CPU

Si tu ADC transmite a varios kilohercios (kHz), acumularás millones de puntos en pocos minutos. Un monitor común solo tiene entre 1000 y 4000 píxeles de ancho físico. Pintar 500,000 puntos en un espacio de 1000 píxeles es un desperdicio, porque muchos puntos caerán exactamente en la misma columna de píxeles tapándose entre sí.Para solucionarlo, si una serie tiene más de, por ejemplo, 5,000 puntos en la ventana actual, ejecuta un algoritmo rápido en la CPU como el LTTB (Largest Triangle Three Buckets) o un submuestreo por columnas (MinMax) antes de enviar los datos a dibujar:Divides la pantalla visible en $N$ columnas (ej: $N = 2000$).Para cada columna de datos reales, quédate únicamente con el punto mínimo y el máximo.De esta manera, reduces un set de un millón de puntos a solo 4,000 puntos manteniendo perfectamente los picos de ruido del ADC y el renderizado será instantáneo.