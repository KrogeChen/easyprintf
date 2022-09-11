EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A3 16535 11693
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
L kicad_symbol_general:HC32L136K8TA_LQFP64 U?
U 1 1 631D5DC1
P 8480 5150
F 0 "U?" H 10208 5196 50  0000 L CNN
F 1 "HC32L136K8TA_LQFP64" H 10208 5105 50  0000 L CNB
F 2 "kicad_footprint_general:LQFP-64_10x10mm_P0.5mm" H 8580 6500 50  0001 C CNN
F 3 "" H 8380 3950 50  0001 C CNN
	1    8480 5150
	1    0    0    -1  
$EndComp
$Comp
L kicad_symbol_general:Crystal_GND24_Small Y10
U 1 1 631D7FDA
P 6135 4800
F 0 "Y10" V 6181 4698 50  0000 R CNN
F 1 "16M" V 6090 4698 50  0000 R CNN
F 2 "kicad_footprint_general:Crystal_SMD_3225-4Pin_3.2x2.5mm" H 6135 4800 50  0001 C CNN
F 3 "~" H 6135 4800 50  0001 C CNN
	1    6135 4800
	0    -1   -1   0   
$EndComp
Wire Wire Line
	5955 4900 6135 4900
Connection ~ 6135 4900
Wire Wire Line
	6135 4900 6830 4900
Wire Wire Line
	6830 4800 6400 4800
Wire Wire Line
	6400 4800 6400 4700
Wire Wire Line
	6400 4700 6135 4700
Connection ~ 6135 4700
Wire Wire Line
	6135 4700 5955 4700
$Comp
L kicad_symbol_general:C_Small C11
U 1 1 631E08E6
P 5855 4900
F 0 "C11" V 5905 5025 50  0000 C CNN
F 1 "9pF" V 5975 5025 50  0000 C CNN
F 2 "kicad_footprint_general:C_0603_1608Metric" H 5855 4900 50  0001 C CNN
F 3 "" H 5855 4900 50  0001 C CNN
	1    5855 4900
	0    1    1    0   
$EndComp
$Comp
L kicad_symbol_general:C_Small C10
U 1 1 631E1B99
P 5855 4700
F 0 "C10" V 5815 4810 50  0000 C CNN
F 1 "9pF" V 5725 4805 50  0000 C CNN
F 2 "kicad_footprint_general:C_0603_1608Metric" H 5855 4700 50  0001 C CNN
F 3 "" H 5855 4700 50  0001 C CNN
	1    5855 4700
	0    1    1    0   
$EndComp
Wire Wire Line
	5755 4700 5700 4700
Wire Wire Line
	5700 4700 5700 4800
Wire Wire Line
	5700 4900 5755 4900
Wire Wire Line
	6035 4830 6010 4830
Wire Wire Line
	6010 4830 6010 4800
Wire Wire Line
	6010 4770 6035 4770
Wire Wire Line
	6010 4800 5700 4800
Connection ~ 6010 4800
Wire Wire Line
	6010 4800 6010 4770
Connection ~ 5700 4800
Wire Wire Line
	5700 4800 5700 4900
$Comp
L power:GND #PWR?
U 1 1 631E29C6
P 5700 4950
F 0 "#PWR?" H 5700 4700 50  0001 C CNN
F 1 "GND" H 5705 4777 50  0000 C CNN
F 2 "" H 5700 4950 50  0001 C CNN
F 3 "" H 5700 4950 50  0001 C CNN
	1    5700 4950
	1    0    0    -1  
$EndComp
Wire Wire Line
	5700 4900 5700 4950
Connection ~ 5700 4900
$EndSCHEMATC
