# Procedimiento para programar en masa

Repetir los siguientes pasos para cada logger:
1. Con un alicate cortar los pines del lado de la pila.
2. Ubicar el logger en el adaptador de la placa de desarrollo MSP, tal que los pines del adaptador hagan contacto con los casselated holes del logger.
3. Actualizar "Penguin Number" incrementando de a 1.
4. Escribir el número sobre el logger
5. Colocar pila, chequear polaridad.
6. Cargar el programa en el logger.
7. Conectar logger a la base con los ganchitos.
8. Correr script logger_test_1.py y revisar resultados
9. Dejar 5 minutos midiendo, con las patas en CC
10. Correr script logger_test_2.py y revisar resultados y eventos (chequear que no hayan resets) 

Se recomienda realizar este procedimiento paralelizando con 2 bases y 1 placa de desarrollo MSP.

# Procedimiento para poxipolear en masa

Repetir los siguientes pasos:
1. Sacar 5 pilas del blister
2. Colocar cada pila en un logger, chequear polaridad
3. Medir con el tester Vcc con GND -> Debería dar ~3.3V -> Si da menos de 3.20V, cambiarla
5. Preparar poxipol CHIQUITO
4. Pegar solapa con poxipol, intentar no dejar burbujas
5. Medir con el tester Vcc con GND -> Debería dar ~3V
6. Esperar a que seque
7. Correr script comms_test.py
8. Rezar

Se recomienda desconectar el adaptador de la placa de desarrollo MSP y usarlo para medir fácilmente la tensión entre Vcc y GND.