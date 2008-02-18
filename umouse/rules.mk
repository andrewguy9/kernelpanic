#map units
umouse/$(TARGET)_map.o: umouse/map.c umouse/map.h

umouse/maptest.o: umouse/maptest.c

umouse/$(TARGET)_floodfill.o: umouse/floodfill.c umouse/floodfill.h

umouse/floodfilltest.o:	umouse/floodfilltest.c

umouse/$(TARGET)_move.o: umouse/move.c umouse/move.h

umouse/movetest.o: umouse/movetest.c

umouse/$(TARGET)_positionlog.o: umouse/positionlog.c umouse/positionlog.h

#motor control
umouse/$(TARGET)_motor.o: umouse/motor.c umouse/motor.h

umouse/motortest.o: umouse/motortest.c

#actual umouse exe
umouse/main.o: umouse/main.c
