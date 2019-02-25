SERIALNO=$1
GENERAL_SHELL=$2
PLATFORM=$3


echo "Adjust power to performance mode on $SERIALNO, $PLATFORM"

if [ "$GENERAL_SHELL" == "adb"* ]; then
    ADB="adb -s $SERIALNO"
    $ADB root || exit 1
    $ADB wait-for-device
    $ADB remount
    $ADB wait-for-device
fi

$GENERAL_SHELL "stop thermald"
$GENERAL_SHELL "stop mpdecision"
# disable thermal
$GENERAL_SHELL "stop thermal-engine && stop thermal-hal-1-0"
# stop perflock HAL
$GENERAL_SHELL "stop perf-hal-1-0"

# boost cpu freq
$GENERAL_SHELL "echo 1 > /sys/devices/system/cpu/cpu0/online"
$GENERAL_SHELL "echo 1 > /sys/devices/system/cpu/cpu1/online"
$GENERAL_SHELL "echo 1 > /sys/devices/system/cpu/cpu2/online"
$GENERAL_SHELL "echo 1 > /sys/devices/system/cpu/cpu3/online"
$GENERAL_SHELL "echo 1 > /sys/devices/system/cpu/cpu4/online"
$GENERAL_SHELL "echo 1 > /sys/devices/system/cpu/cpu5/online"
$GENERAL_SHELL "echo 1 > /sys/devices/system/cpu/cpu6/online"
$GENERAL_SHELL "echo 1 > /sys/devices/system/cpu/cpu7/online"

$GENERAL_SHELL "echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
$GENERAL_SHELL "echo performance > /sys/devices/system/cpu/cpu1/cpufreq/scaling_governor"
$GENERAL_SHELL "echo performance > /sys/devices/system/cpu/cpu2/cpufreq/scaling_governor"
$GENERAL_SHELL "echo performance > /sys/devices/system/cpu/cpu3/cpufreq/scaling_governor"
$GENERAL_SHELL "echo performance > /sys/devices/system/cpu/cpu4/cpufreq/scaling_governor"
$GENERAL_SHELL "echo performance > /sys/devices/system/cpu/cpu5/cpufreq/scaling_governor"
$GENERAL_SHELL "echo performance > /sys/devices/system/cpu/cpu6/cpufreq/scaling_governor"
$GENERAL_SHELL "echo performance > /sys/devices/system/cpu/cpu7/cpufreq/scaling_governor"

# bw vote max
$GENERAL_SHELL "echo performance > /sys/class/devfreq/1d84000.ufshc/governor"
$GENERAL_SHELL "echo performance > /sys/class/devfreq/5000000.qcom,kgsl-3d0/governor"
$GENERAL_SHELL "echo performance > /sys/class/devfreq/aa00000.qcom,vidc:arm9_bus_ddr/governor"
$GENERAL_SHELL "echo performance > /sys/class/devfreq/aa00000.qcom,vidc:bus_cnoc/governor"
$GENERAL_SHELL "echo performance > /sys/class/devfreq/aa00000.qcom,vidc:venus_bus_ddr/governor"
$GENERAL_SHELL "echo performance > /sys/class/devfreq/aa00000.qcom,vidc:venus_bus_llcc/governor"
$GENERAL_SHELL "echo performance > /sys/class/devfreq/soc:qcom,cpubw/governor"
$GENERAL_SHELL "echo performance > /sys/class/devfreq/soc:qcom,gpubw/governor"
$GENERAL_SHELL "echo performance > /sys/class/devfreq/soc:qcom,kgsl-busmon/governor"
$GENERAL_SHELL "echo performance > /sys/class/devfreq/soc:qcom,l3-cdsp/governor"
$GENERAL_SHELL "echo performance > /sys/class/devfreq/soc:qcom,l3-cpu0/governor"
$GENERAL_SHELL "echo performance > /sys/class/devfreq/soc:qcom,l3-cpu4/governor"
$GENERAL_SHELL "echo performance > /sys/class/devfreq/soc:qcom,llccbw/governor"
$GENERAL_SHELL "echo performance > /sys/class/devfreq/soc:qcom,memlat-cpu0/governor"
$GENERAL_SHELL "echo performance > /sys/class/devfreq/soc:qcom,memlat-cpu4/governor"
$GENERAL_SHELL "echo performance > /sys/class/devfreq/soc:qcom,mincpubw/governor"
$GENERAL_SHELL "echo performance > /sys/class/devfreq/soc:qcom,snoc_cnoc_keepalive/governor"

# boost gpu freq
$GENERAL_SHELL "echo 0 > /sys/class/kgsl/kgsl-3d0/min_pwrlevel"
$GENERAL_SHELL "echo 0 > /sys/class/kgsl/kgsl-3d0/max_pwrlevel"
$GENERAL_SHELL "echo performance > /sys/class/kgsl/kgsl-3d0/devfreq/governor"
$GENERAL_SHELL "cat /sys/class/kgsl/kgsl-3d0/gpuclk"
$GENERAL_SHELL "echo 1000000 > /sys/class/kgsl/kgsl-3d0/idle_timer"
$GENERAL_SHELL "echo 1 > /d/dri/0/debug/core_perf/perf_mode"


$GENERAL_SHELL "echo 4 > /sys/devices/system/cpu/cpu0/core_ctl/min_cpus"
$GENERAL_SHELL "echo 4 > /sys/devices/system/cpu/cpu4/core_ctl/min_cpus"
$GENERAL_SHELL "echo 35 > /proc/sys/kernel/sched_downmigrate && echo 55 > /proc/sys/kernel/sched_upmigrate"
$GENERAL_SHELL "echo 512 > /sys/block/sda/queue/nr_requests && echo 1024 > /sys/block/sda/queue/read_ahead_kb"

#$GENERAL_SHELL "echo 100 > /proc/sys/kernel/sched_cfs_boost"
$GENERAL_SHELL "echo 100 > /dev/stune/top-app/schedtune.boost"
$GENERAL_SHELL "echo 1 > /dev/stune/top-app/schedtune.prefer_idle"

# disable all level LPM by sysfs node
$GENERAL_SHELL "echo Y > /sys/module/lpm_levels/parameters/sleep_disabled"
$GENERAL_SHELL "echo N > /sys/module/lpm_levels/L3/cpu0/pc/idle_enabled"
$GENERAL_SHELL "echo N > /sys/module/lpm_levels/L3/cpu0/rail-pc/idle_enabled"
$GENERAL_SHELL "echo N > /sys/module/lpm_levels/L3/cpu1/pc/idle_enabled"
$GENERAL_SHELL "echo N > /sys/module/lpm_levels/L3/cpu1/rail-pc/idle_enabled"
$GENERAL_SHELL "echo N > /sys/module/lpm_levels/L3/cpu2/pc/idle_enabled"
$GENERAL_SHELL "echo N > /sys/module/lpm_levels/L3/cpu2/rail-pc/idle_enabled"
$GENERAL_SHELL "echo N > /sys/module/lpm_levels/L3/cpu3/pc/idle_enabled"
$GENERAL_SHELL "echo N > /sys/module/lpm_levels/L3/cpu3/rail-pc/idle_enabled"
$GENERAL_SHELL "echo N > /sys/module/lpm_levels/L3/cpu4/pc/idle_enabled"
$GENERAL_SHELL "echo N > /sys/module/lpm_levels/L3/cpu4/rail-pc/idle_enabled"
$GENERAL_SHELL "echo N > /sys/module/lpm_levels/L3/cpu5/pc/idle_enabled"
$GENERAL_SHELL "echo N > /sys/module/lpm_levels/L3/cpu5/rail-pc/idle_enabled"
$GENERAL_SHELL "echo N > /sys/module/lpm_levels/L3/cpu6/pc/idle_enabled"
$GENERAL_SHELL "echo N > /sys/module/lpm_levels/L3/cpu6/rail-pc/idle_enabled"
$GENERAL_SHELL "echo N > /sys/module/lpm_levels/L3/cpu7/pc/idle_enabled"
$GENERAL_SHELL "echo N > /sys/module/lpm_levels/L3/cpu7/rail-pc/idle_enabled"
$GENERAL_SHELL "echo N > /sys/module/lpm_levels/L3/l3-wfi/idle_enabled"
$GENERAL_SHELL "echo N > /sys/module/lpm_levels/L3/llcc-off/idle_enabled"
if [ "$PLATFORM" == "sdm660" ]; then
	$GENERAL_SHELL "echo N > /sys/module/lpm_levels/system/perf/cpu4/pc/idle_enabled"
	$GENERAL_SHELL "echo N > /sys/module/lpm_levels/system/perf/cpu5/pc/idle_enabled"
	$GENERAL_SHELL "echo N > /sys/module/lpm_levels/system/perf/cpu6/pc/idle_enabled"
	$GENERAL_SHELL "echo N > /sys/module/lpm_levels/system/perf/cpu7/pc/idle_enabled"
fi

# set ddr config.
$GENERAL_SHELL "echo 100 > /proc/sys/kernel/sched_initial_task_util"
if [ "$PLATFORM" == "sdm660" ]; then
	$GENERAL_SHELL "echo 100 > /proc/sys/kernel/sched_init_task_load"  # for 660
	$GENERAL_SHELL "echo 1 >/sys/kernel/debug/msm-bus-dbg/shell-client/mas"
	$GENERAL_SHELL "echo 512 > /sys/kernel/debug/msm-bus-dbg/shell-client/slv"
	$GENERAL_SHELL "echo 28864000000 > /sys/kernel/debug/msm-bus-dbg/shell-client/ab"
	$GENERAL_SHELL "echo 28864000000 > /sys/kernel/debug/msm-bus-dbg/shell-client/ib"
	$GENERAL_SHELL "echo 1 > /sys/kernel/debug/msm-bus-dbg/shell-client/update_request"
fi
