# THIS FILE IS AUTOMATICALLY GENERATED
# Project: C:\Users\leo40\OneDrive\Empresa\Proyectos\Orange_Pi\Carro_Taller\Prueba_USB\V1.0\FC_Impresion.cydsn\FC_Impresion.cyprj
# Date: Wed, 08 Aug 2018 17:12:47 GMT
#set_units -time ns
create_clock -name {CyILO} -period 10000 -waveform {0 5000} [list [get_pins {ClockBlock/ilo}] [get_pins {ClockBlock/clk_100k}] [get_pins {ClockBlock/clk_1k}] [get_pins {ClockBlock/clk_32k}]]
create_clock -name {CyIMO} -period 41.666666666666664 -waveform {0 20.8333333333333} [list [get_pins {ClockBlock/imo}]]
create_clock -name {CyPLL_OUT} -period 15.151515151515152 -waveform {0 7.57575757575758} [list [get_pins {ClockBlock/pllout}]]
create_clock -name {CyMASTER_CLK} -period 15.151515151515152 -waveform {0 7.57575757575758} [list [get_pins {ClockBlock/clk_sync}]]
create_generated_clock -name {CyBUS_CLK} -source [get_pins {ClockBlock/clk_sync}] -edges {1 2 3} [list [get_pins {ClockBlock/clk_bus_glb}]]
create_generated_clock -name {PSOC_IntClock} -source [get_pins {ClockBlock/clk_sync}] -edges {1 73 145} [list [get_pins {ClockBlock/dclk_glb_0}]]
create_generated_clock -name {RS232_IntClock} -source [get_pins {ClockBlock/clk_sync}] -edges {1 73 145} [list [get_pins {ClockBlock/dclk_glb_1}]]
create_generated_clock -name {Bluetooth_IntClock} -source [get_pins {ClockBlock/clk_sync}] -edges {1 859 1719} [list [get_pins {ClockBlock/dclk_glb_2}]]
create_generated_clock -name {RFID_IntClock} -source [get_pins {ClockBlock/clk_sync}] -edges {1 859 1719} [list [get_pins {ClockBlock/dclk_glb_3}]]
create_generated_clock -name {Clock_1} -source [get_pins {ClockBlock/clk_sync}] -edges {1 1435 2871} [list [get_pins {ClockBlock/dclk_glb_4}]]
create_generated_clock -name {Surtidor_IntClock} -source [get_pins {ClockBlock/clk_sync}] -edges {1 1719 3439} [list [get_pins {ClockBlock/dclk_glb_5}]]


# Component constraints for C:\Users\leo40\OneDrive\Empresa\Proyectos\Orange_Pi\Carro_Taller\Prueba_USB\V1.0\FC_Impresion.cydsn\TopDesign\TopDesign.cysch
# Project: C:\Users\leo40\OneDrive\Empresa\Proyectos\Orange_Pi\Carro_Taller\Prueba_USB\V1.0\FC_Impresion.cydsn\FC_Impresion.cyprj
# Date: Wed, 08 Aug 2018 17:12:39 GMT