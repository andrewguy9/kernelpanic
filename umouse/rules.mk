#map units
umouse/$(TARGET)_map.o: umouse/map.c umouse/map.h

umouse/$(TARGET)_maptest.o: umouse/maptest.c

umouse/$(TARGET)_floodfill.o: umouse/floodfill.c umouse/floodfill.h

umouse/$(TARGET)_floodfilltest.o:	umouse/floodfilltest.c

umouse/$(TARGET)_move.o: umouse/move.c umouse/move.h

umouse/$(TARGET)_movetest.o: umouse/movetest.c

umouse/$(TARGET)_positionlog.o: umouse/positionlog.c umouse/positionlog.h

umouse/$(TARGET)_submove.o: umouse/submove.c umouse/submove.h

#motor control
umouse/$(TARGET)_motor.o: umouse/motor.c umouse/motor.h

umouse/$(TARGET)_motortest.o: umouse/motortest.c

#actual umouse exe
umouse/$(TARGET)_main.o: umouse/main.c
