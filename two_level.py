
from caches import *
import m5
from m5.objects import *
from m5.util import convert

import x86_mp

system = System()
 
system.clk_domain = SrcClockDomain()
system.clk_domain.clock = '2GHz'
system.clk_domain.voltage_domain = VoltageDomain()
 
#binary = 'configs/tutorial/project/WriteEfficientTiling/out'
 
#system.workload = SEWorkload.init_compatible(binary)
 
#process = Process()
#process.cmd = [binary]

system.mem_mode = 'timing'
system.mem_ranges = [AddrRange('512MB')]

# Memory
system.membus = SystemXBar()
system.membus.badaddr_responder = BadAddr()
system.membus.default = system.membus.badaddr_responder.pio
# Set up the system port for functional access from the simulator
system.system_port = system.membus.cpu_side_ports

x86_mp.init_fs(system, system.membus, 8)
system.kernel = binary

#Creat CPU
system.cpu1 = O3CPU()
#system.cpu1.workload = process
system.cpu1.createThreads()

system.cpu2 = O3CPU()
#system.cpu2.workload = process
system.cpu2.createThreads()

system.cpu3 = O3CPU()
#system.cpu3.workload = process
system.cpu3.createThreads()

system.cpu4 = O3CPU()
#system.cpu4.workload = process
system.cpu4.createThreads()

system.cpu5 = O3CPU()
#system.cpu5.workload = process
system.cpu5.createThreads()

system.cpu6 = O3CPU()
#system.cpu6.workload = process
system.cpu6.createThreads()

system.cpu7 = O3CPU()
#system.cpu7.workload = process
system.cpu7.createThreads()

system.cpu8 = O3CPU()
#system.cpu8.workload = process
system.cpu8.createThreads()


#Create cache hierarchy
system.cpu1.icache = L1ICache()
system.cpu1.dcache = L1DCache()

system.cpu2.icache = L1ICache()
system.cpu2.dcache = L1DCache()

system.cpu3.icache = L1ICache()
system.cpu3.dcache = L1DCache()

system.cpu4.icache = L1ICache()
system.cpu4.dcache = L1DCache()

system.cpu5.icache = L1ICache()
system.cpu5.dcache = L1DCache()

system.cpu6.icache = L1ICache()
system.cpu6.dcache = L1DCache()

system.cpu7.icache = L1ICache()
system.cpu7.dcache = L1DCache()

system.cpu8.icache = L1ICache()
system.cpu8.dcache = L1DCache()

#Connect cpu to own L1 caches
system.cpu1.icache.connectCPU(system.cpu1)
system.cpu1.dcache.connectCPU(system.cpu1)

system.cpu2.icache.connectCPU(system.cpu2)
system.cpu2.dcache.connectCPU(system.cpu2)

system.cpu3.icache.connectCPU(system.cpu3)
system.cpu3.dcache.connectCPU(system.cpu3)

system.cpu4.icache.connectCPU(system.cpu4)
system.cpu4.dcache.connectCPU(system.cpu4)

system.cpu5.icache.connectCPU(system.cpu5)
system.cpu5.dcache.connectCPU(system.cpu5)

system.cpu6.icache.connectCPU(system.cpu6)
system.cpu6.dcache.connectCPU(system.cpu6)

system.cpu7.icache.connectCPU(system.cpu7)
system.cpu7.dcache.connectCPU(system.cpu7)

system.cpu8.icache.connectCPU(system.cpu8)
system.cpu8.dcache.connectCPU(system.cpu8)
'''
#Connect the CPU TLBs directly to the mem.
system.cpu1.itb.walker.port = system.membus.cpu_side_ports
system.cpu1.dtb.walker.port = system.membus.cpu_side_ports

system.cpu2.itb.walker.port = system.membus.cpu_side_ports
system.cpu2.dtb.walker.port = system.membus.cpu_side_ports

system.cpu3.itb.walker.port = system.membus.cpu_side_ports
system.cpu3.dtb.walker.port = system.membus.cpu_side_ports

system.cpu4.itb.walker.port = system.membus.cpu_side_ports
system.cpu4.dtb.walker.port = system.membus.cpu_side_ports

system.cpu5.itb.walker.port = system.membus.cpu_side_ports
system.cpu5.dtb.walker.port = system.membus.cpu_side_ports

system.cpu6.itb.walker.port = system.membus.cpu_side_ports
system.cpu6.dtb.walker.port = system.membus.cpu_side_ports

system.cpu7.itb.walker.port = system.membus.cpu_side_ports
system.cpu7.dtb.walker.port = system.membus.cpu_side_ports

system.cpu8.itb.walker.port = system.membus.cpu_side_ports
system.cpu8.dtb.walker.port = system.membus.cpu_side_ports
'''
#Create L2 bus
system.l2bus = L2XBar()
system.cpu1.icache.connectBus(system.l2bus)
system.cpu1.dcache.connectBus(system.l2bus)

system.cpu2.icache.connectBus(system.l2bus)
system.cpu2.dcache.connectBus(system.l2bus)

system.cpu3.icache.connectBus(system.l2bus)
system.cpu3.dcache.connectBus(system.l2bus)

system.cpu4.icache.connectBus(system.l2bus)
system.cpu4.dcache.connectBus(system.l2bus)

system.cpu5.icache.connectBus(system.l2bus)
system.cpu5.dcache.connectBus(system.l2bus)

system.cpu6.icache.connectBus(system.l2bus)
system.cpu6.dcache.connectBus(system.l2bus)

system.cpu7.icache.connectBus(system.l2bus)
system.cpu7.dcache.connectBus(system.l2bus)

system.cpu8.icache.connectBus(system.l2bus)
system.cpu8.dcache.connectBus(system.l2bus)

#Create L2 cache
system.l2cache = L2Cache()
system.l2cache.connectCPUSideBus(system.l2bus)
system.l2cache.connectMemSideBus(system.membus)

#Create the interrupt controller for the CPU
system.cpu1.createInterruptController()
system.cpu2.createInterruptController()
system.cpu3.createInterruptController()
system.cpu4.createInterruptController()
system.cpu5.createInterruptController()
system.cpu6.createInterruptController()
system.cpu7.createInterruptController()
system.cpu8.createInterruptController()

#system.system_port = system.membus.cpu_side_ports

if m5.defines.buildEnv['TARGET_ISA'] == "x86":
    system.cpu1.interrupts[0].pio = system.membus.mem_side_ports
    system.cpu1.interrupts[0].int_requestor = system.membus.cpu_side_ports
    system.cpu1.interrupts[0].int_responder = system.membus.mem_side_ports
    
    system.cpu2.interrupts[0].pio = system.membus.mem_side_ports
    system.cpu2.interrupts[0].int_requestor = system.membus.cpu_side_ports
    system.cpu2.interrupts[0].int_responder = system.membus.mem_side_ports
    
    system.cpu3.interrupts[0].pio = system.membus.mem_side_ports
    system.cpu3.interrupts[0].int_requestor = system.membus.cpu_side_ports
    system.cpu3.interrupts[0].int_responder = system.membus.mem_side_ports
    
    system.cpu4.interrupts[0].pio = system.membus.mem_side_ports
    system.cpu4.interrupts[0].int_requestor = system.membus.cpu_side_ports
    system.cpu4.interrupts[0].int_responder = system.membus.mem_side_ports
    
    system.cpu5.interrupts[0].pio = system.membus.mem_side_ports
    system.cpu5.interrupts[0].int_requestor = system.membus.cpu_side_ports
    system.cpu5.interrupts[0].int_responder = system.membus.mem_side_ports
    
    system.cpu6.interrupts[0].pio = system.membus.mem_side_ports
    system.cpu6.interrupts[0].int_requestor = system.membus.cpu_side_ports
    system.cpu6.interrupts[0].int_responder = system.membus.mem_side_ports
    
    system.cpu7.interrupts[0].pio = system.membus.mem_side_ports
    system.cpu7.interrupts[0].int_requestor = system.membus.cpu_side_ports
    system.cpu7.interrupts[0].int_responder = system.membus.mem_side_ports
    
    system.cpu8.interrupts[0].pio = system.membus.mem_side_ports
    system.cpu8.interrupts[0].int_requestor = system.membus.cpu_side_ports
    system.cpu8.interrupts[0].int_responder = system.membus.mem_side_ports
 

system.mem_ctrl = MemCtrl()
system.mem_ctrl.port = system.membus.mem_side_ports
system.mem_ctrl.dram = DDR3_1600_8x8()
system.mem_ctrl.dram.range = system.mem_ranges[0]
 

 

root = Root(full_system = True,system = system)
m5.instantiate()
 

print("Beginning simulation")
exit_event = m5.simulate()
 

print('Exiting @ tick {} because {}'
    .format(m5.curTick(),exit_event.getCause()))
