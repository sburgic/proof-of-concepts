EESchema Schematic File Version 4
LIBS:plant_water_level-cache
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Device:Battery_Cell BAT1
U 1 1 5FE8A885
P 1650 1600
F 0 "BAT1" H 1768 1696 50  0000 L CNN
F 1 "CR2032" H 1768 1605 50  0000 L CNN
F 2 "Battery:BatteryHolder_Keystone_103_1x20mm" V 1650 1660 50  0001 C CNN
F 3 "~" V 1650 1660 50  0001 C CNN
	1    1650 1600
	1    0    0    -1  
$EndComp
$Comp
L power:+3V0 #PWR01
U 1 1 5FE8B25F
P 850 1350
F 0 "#PWR01" H 850 1200 50  0001 C CNN
F 1 "+3V0" H 865 1523 50  0000 C CNN
F 2 "" H 850 1350 50  0001 C CNN
F 3 "" H 850 1350 50  0001 C CNN
	1    850  1350
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR02
U 1 1 5FE8B753
P 1650 1800
F 0 "#PWR02" H 1650 1550 50  0001 C CNN
F 1 "GND" H 1655 1627 50  0000 C CNN
F 2 "" H 1650 1800 50  0001 C CNN
F 3 "" H 1650 1800 50  0001 C CNN
	1    1650 1800
	1    0    0    -1  
$EndComp
Wire Wire Line
	1650 1700 1650 1800
$Comp
L Transistor_BJT:BC547 Q1
U 1 1 5FE8C080
P 3650 2400
F 0 "Q1" H 3841 2446 50  0000 L CNN
F 1 "BC547" H 3841 2355 50  0000 L CNN
F 2 "Package_TO_SOT_THT:TO-92_Inline" H 3850 2325 50  0001 L CIN
F 3 "http://www.fairchildsemi.com/ds/BC/BC547.pdf" H 3650 2400 50  0001 L CNN
	1    3650 2400
	1    0    0    -1  
$EndComp
$Comp
L Device:LED D1
U 1 1 5FE8D472
P 3750 1950
F 0 "D1" V 3789 1833 50  0000 R CNN
F 1 "LED" V 3698 1833 50  0000 R CNN
F 2 "LED_THT:LED_D5.0mm" H 3750 1950 50  0001 C CNN
F 3 "~" H 3750 1950 50  0001 C CNN
	1    3750 1950
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3750 2100 3750 2200
$Comp
L power:+3V0 #PWR03
U 1 1 5FE8E2AC
P 3750 1300
F 0 "#PWR03" H 3750 1150 50  0001 C CNN
F 1 "+3V0" H 3765 1473 50  0000 C CNN
F 2 "" H 3750 1300 50  0001 C CNN
F 3 "" H 3750 1300 50  0001 C CNN
	1    3750 1300
	1    0    0    -1  
$EndComp
Wire Wire Line
	3750 1300 3750 1350
$Comp
L power:GND #PWR04
U 1 1 5FE91780
P 3750 2700
F 0 "#PWR04" H 3750 2450 50  0001 C CNN
F 1 "GND" H 3755 2527 50  0000 C CNN
F 2 "" H 3750 2700 50  0001 C CNN
F 3 "" H 3750 2700 50  0001 C CNN
	1    3750 2700
	1    0    0    -1  
$EndComp
Wire Wire Line
	3750 2600 3750 2650
$Comp
L Device:R R1
U 1 1 5FE922AF
P 3300 1750
F 0 "R1" H 3370 1796 50  0000 L CNN
F 1 "220K" H 3370 1705 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 3230 1750 50  0001 C CNN
F 3 "~" H 3300 1750 50  0001 C CNN
	1    3300 1750
	1    0    0    -1  
$EndComp
Wire Wire Line
	3750 1350 3300 1350
Wire Wire Line
	3300 1350 3300 1600
Connection ~ 3750 1350
Wire Wire Line
	3300 1900 3300 2400
Wire Wire Line
	3300 2400 3450 2400
Connection ~ 3300 2400
Connection ~ 3750 2650
Wire Wire Line
	3750 2650 3750 2700
Wire Wire Line
	3750 1350 3750 1800
$Comp
L Switch:SW_SPDT SW1
U 1 1 5FE9A194
P 1250 1500
F 0 "SW1" H 1250 1785 50  0000 C CNN
F 1 "SW_SPDT" H 1250 1694 50  0000 C CNN
F 2 "Button_Switch_SMD:SW_SPDT_PCM12" H 1250 1500 50  0001 C CNN
F 3 "~" H 1250 1500 50  0001 C CNN
	1    1250 1500
	1    0    0    -1  
$EndComp
Wire Wire Line
	850  1350 850  1500
Wire Wire Line
	850  1500 1050 1500
$Comp
L Connector:TestPoint TP1
U 1 1 6004CAAA
P 3050 2400
F 0 "TP1" V 3153 2472 50  0000 C CNN
F 1 "Probe2" H 3108 2427 50  0001 L CNN
F 2 "TestPoint:TestPoint_THTPad_4.0x4.0mm_Drill2.0mm" H 3250 2400 50  0001 C CNN
F 3 "~" H 3250 2400 50  0001 C CNN
	1    3050 2400
	0    -1   -1   0   
$EndComp
$Comp
L Connector:TestPoint TP2
U 1 1 6004CE1E
P 3050 2650
F 0 "TP2" V 3153 2722 50  0000 C CNN
F 1 "Probe1" H 3108 2677 50  0001 L CNN
F 2 "TestPoint:TestPoint_THTPad_4.0x4.0mm_Drill2.0mm" H 3250 2650 50  0001 C CNN
F 3 "~" H 3250 2650 50  0001 C CNN
	1    3050 2650
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3050 2650 3100 2650
Wire Wire Line
	3050 2400 3100 2400
$Comp
L Connector:TestPoint TP3
U 1 1 60052D37
P 2800 2400
F 0 "TP3" V 2903 2472 50  0000 C CNN
F 1 "Probe2" H 2858 2427 50  0001 L CNN
F 2 "TestPoint:TestPoint_THTPad_4.0x4.0mm_Drill2.0mm" H 3000 2400 50  0001 C CNN
F 3 "~" H 3000 2400 50  0001 C CNN
	1    2800 2400
	0    -1   -1   0   
$EndComp
$Comp
L Connector:TestPoint TP4
U 1 1 60052F42
P 2800 2650
F 0 "TP4" V 2903 2722 50  0000 C CNN
F 1 "Probe2" H 2858 2677 50  0001 L CNN
F 2 "TestPoint:TestPoint_THTPad_4.0x4.0mm_Drill2.0mm" H 3000 2650 50  0001 C CNN
F 3 "~" H 3000 2650 50  0001 C CNN
	1    2800 2650
	0    -1   -1   0   
$EndComp
Wire Wire Line
	2800 2400 2850 2400
Wire Wire Line
	2850 2400 2850 2250
Wire Wire Line
	2850 2250 3100 2250
Wire Wire Line
	3100 2250 3100 2400
Connection ~ 3100 2400
Wire Wire Line
	3100 2400 3300 2400
Wire Wire Line
	2800 2650 2850 2650
Wire Wire Line
	2850 2650 2850 2500
Wire Wire Line
	2850 2500 3100 2500
Wire Wire Line
	3100 2500 3100 2650
Connection ~ 3100 2650
Wire Wire Line
	3100 2650 3750 2650
Wire Wire Line
	1450 1400 1650 1400
$EndSCHEMATC
