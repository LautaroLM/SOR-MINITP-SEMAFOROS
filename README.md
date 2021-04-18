# SOR-Semáforos-1S-2021
Trabajo Práctico Semáforos primer semestre año 2021


ACCIONES DE SUBWAY ARGENTO:
-Cortar ajo y perejil
-Mezclar los ingredientes anteriores con 1 huevo
-Salar
-Agregar la carne y empanar
-Cocinar en sartén
-Hornear pan
-Cortar lechuga fresca, tomate, cebolla morada y pepino
-Armar sandwich

PSEUDOCÓDIGO:

Cortar()
  V(s_mezclar)

P(s_mezclar)
Mezclar()
  V(s_salar)

P(s_salar)
Mutex()
  Salar()
Mutex()
V(s_agregarCarneEmpanar)

P(s_agregarCarneEmpanar)
agregarCarneEmpanar()
  V(s_cocinarSarten)

P(s_cocinarSarten)
Mutex()
  cocinarSarten()
Mutex()

Mutex()
hornearPan()
Mutex()

cortarExtras()
  V(s_armarSandwich)

P(s_armarSandwich)
armarSandwich()



