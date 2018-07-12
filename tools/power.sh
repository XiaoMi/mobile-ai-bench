SERIALNO=$1
PLATFORM=$2
ADB="adb -s $SERIALNO"

echo "Adjust power to performance mode on $SERIALNO, $PLATFORM"

$ADB root || exit 1
$ADB wait-for-device
$ADB remount
$ADB wait-for-device

$ADB shell "stop thermald"
$ADB shell "stop mpdecision"
# disable thermal
$ADB shell "stop thermal-engine && stop thermal-hal-1-0"
# stop perflock HAL
$ADB shell "stop perf-hal-1-0"

# boost cpu freq
$ADB shell "echo 1 > /sys/devices/system/cpu/cpu0/online"
$ADB shell "echo 1 > /sys/devices/system/cpu/cpu1/online"
$ADB shell "echo 1 > /sys/devices/system/cpu/cpu2/online"
$ADB shell "echo 1 > /sys/devices/system/cpu/cpu3/online"
$ADB shell "echo 1 > /sys/devices/system/cpu/cpu4/online"
$ADB shell "echo 1 > /sys/devices/system/cpu/cpu5/online"
$ADB shell "echo 1 > /sys/devices/system/cpu/cpu6/online"
$ADB shell "echo 1 > /sys/devices/system/cpu/cpu7/online"

$ADB shell "echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
$ADB shell "echo performance > /sys/devices/system/cpu/cpu1/cpufreq/scaling_governor"
$ADB shell "echo performance > /sys/devices/system/cpu/cpu2/cpufreq/scaling_governor"
$ADB shell "echo performance > /sys/devices/system/cpu/cpu3/cpufreq/scaling_governor"
$ADB shell "echo performance > /sys/devices/system/cpu/cpu4/cpufreq/scaling_governor"
$ADB shell "echo performance > /sys/devices/system/cpu/cpu5/cpufreq/scaling_governor"
$ADB shell "echo performance > /sys/devices/system/cpu/cpu6/cpufreq/scaling_governor"
$ADB shell "echo performance > /sys/devices/system/cpu/cpu7/cpufreq/scaling_governor"

# bw vote max
$ADB shell "echo performance > /sys/class/devfreq/1d84000.ufshc/governor"
$ADB shell "echo performance > /sys/class/devfreq/5000000.qcom,kgsl-3d0/governor"
$ADB shell "echo performance > /sys/class/devfreq/aa00000.qcom,vidc:arm9_bus_ddr/governor"
$ADB shell "echo performance > /sys/class/devfreq/aa00000.qcom,vidc:bus_cnoc/governor"
$ADB shell "echo performance > /sys/class/devfreq/aa00000.qcom,vidc:venus_bus_ddr/governor"
$ADB shell "echo performance > /sys/class/devfreq/aa00000.qcom,vidc:venus_bus_llcc/governor"
$ADB shell "echo performance > /sys/class/devfreq/soc:qcom,cpubw/governor"
$ADB shell "echo performance > /sys/class/devfreq/soc:qcom,gpubw/governor"
$ADB shell "echo performance > /sys/class/devfreq/soc:qcom,kgsl-busmon/governor"
$ADB shell "echo performance > /sys/class/devfreq/soc:qcom,l3-cdsp/governor"
$ADB shell "echo performance > /sys/class/devfreq/soc:qcom,l3-cpu0/governor"
$ADB shell "echo performance > /sys/class/devfreq/soc:qcom,l3-cpu4/governor"
$ADB shell "echo performance > /sys/class/devfreq/soc:qcom,llccbw/governor"
$ADB shell "echo performance > /sys/class/devfreq/soc:qcom,memlat-cpu0/governor"
$ADB shell "echo performance > /sys/class/devfreq/soc:qcom,memlat-cpu4/governor"
$ADB shell "echo performance > /sys/class/devfreq/soc:qcom,mincpubw/governor"
$ADB shell "echo performance > /sys/class/devfreq/soc:qcom,snoc_cnoc_keepalive/governor"

# boost gpu freq
$ADB shell "echo 0 > /sys/class/kgsl/kgsl-3d0/min_pwrlevel"
$ADB shell "echo 0 > /sys/class/kgsl/kgsl-3d0/max_pwrlevel"
$ADB shell "echo performance > /sys/class/kgsl/kgsl-3d0/devfreq/governor"
$ADB shell "cat /sys/class/kgsl/kgsl-3d0/gpuclk"
$ADB shell "echo 1000000 > /sys/class/kgsl/kgsl-3d0/idle_timer"
$ADB shell "echo 1 > /d/dri/0/debug/core_perf/perf_mode"


$ADB shell "echo 4 > /sys/devices/system/cpu/cpu0/core_ctl/min_cpus"
$ADB shell "echo 4 > /sys/devices/system/cpu/cpu4/core_ctl/min_cpus"
$ADB shell "echo 35 > /proc/sys/kernel/sched_downmigrate && echo 55 > /proc/sys/kernel/sched_upmigrate"
$ADB shell "echo 512 > /sys/block/sda/queue/nr_requests && echo 1024 > /sys/block/sda/queue/read_ahead_kb"

#$ADB shell "echo 100 > /proc/sys/kernel/sched_cfs_boost"
$ADB shell "echo 100 > /dev/stune/top-app/schedtune.boost"
$ADB shell "echo 1 > /dev/stune/top-app/schedtune.prefer_idle"

# disable all level LPM by sysfs node
$ADB shell "echo Y > /sys/module/lpm_levels/parameters/sleep_disabled"
$ADB shell "echo N > /sys/module/lpm_levels/L3/cpu0/pc/idle_enabled"
$ADB shell "echo N > /sys/module/lpm_levels/L3/cpu0/rail-pc/idle_enabled"
$ADB shell "echo N > /sys/module/lpm_levels/L3/cpu1/pc/idle_enabled"
$ADB shell "echo N > /sys/module/lpm_levels/L3/cpu1/rail-pc/idle_enabled"
$ADB shell "echo N > /sys/module/lpm_levels/L3/cpu2/pc/idle_enabled"
$ADB shell "echo N > /sys/module/lpm_levels/L3/cpu2/rail-pc/idle_enabled"
$ADB shell "echo N > /sys/module/lpm_levels/L3/cpu3/pc/idle_enabled"
$ADB shell "echo N > /sys/module/lpm_levels/L3/cpu3/rail-pc/idle_enabled"
$ADB shell "echo N > /sys/module/lpm_levels/L3/cpu4/pc/idle_enabled"
$ADB shell "echo N > /sys/module/lpm_levels/L3/cpu4/rail-pc/idle_enabled"
$ADB shell "echo N > /sys/module/lpm_levels/L3/cpu5/pc/idle_enabled"
$ADB shell "echo N > /sys/module/lpm_levels/L3/cpu5/rail-pc/idle_enabled"
$ADB shell "echo N > /sys/module/lpm_levels/L3/cpu6/pc/idle_enabled"
$ADB shell "echo N > /sys/module/lpm_levels/L3/cpu6/rail-pc/idle_enabled"
$ADB shell "echo N > /sys/module/lpm_levels/L3/cpu7/pc/idle_enabled"
$ADB shell "echo N > /sys/module/lpm_levels/L3/cpu7/rail-pc/idle_enabled"
$ADB shell "echo N > /sys/module/lpm_levels/L3/l3-wfi/idle_enabled"
$ADB shell "echo N > /sys/module/lpm_levels/L3/llcc-off/idle_enabled"
if [ "$PLATFORM" == "sdm660" ]; then
	$ADB shell "echo N > /sys/module/lpm_levels/system/perf/cpu4/pc/idle_enabled"
	$ADB shell "echo N > /sys/module/lpm_levels/system/perf/cpu5/pc/idle_enabled"
	$ADB shell "echo N > /sys/module/lpm_levels/system/perf/cpu6/pc/idle_enabled"
	$ADB shell "echo N > /sys/module/lpm_levels/system/perf/cpu7/pc/idle_enabled"
fi

# set ddr config.
$ADB shell "echo 100 > /proc/sys/kernel/sched_initial_task_util"
if [ "$PLATFORM" == "sdm660" ]; then
	$ADB shell "echo 100 > /proc/sys/kernel/sched_init_task_load"  # for 660
	$ADB shell "echo 1 >/sys/kernel/debug/msm-bus-dbg/shell-client/mas"
	$ADB shell "echo 512 > /sys/kernel/debug/msm-bus-dbg/shell-client/slv"
	$ADB shell "echo 28864000000 > /sys/kernel/debug/msm-bus-dbg/shell-client/ab"
	$ADB shell "echo 28864000000 > /sys/kernel/debug/msm-bus-dbg/shell-client/ib"
	$ADB shell "echo 1 > /sys/kernel/debug/msm-bus-dbg/shell-client/update_request"
fi