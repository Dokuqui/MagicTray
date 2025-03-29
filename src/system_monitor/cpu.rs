use anyhow::Result;
use sysinfo::{System, RefreshKind, CpuRefreshKind};
use std::time::Duration;
use std::thread::sleep;

pub fn get_cpu_usage() -> Result<f64> {
    static mut FIRST_CALL: bool = true;
    static mut PREV_USAGE: f64 = 0.0;

    let mut sys = System::new_with_specifics(
        RefreshKind::everything().with_cpu(CpuRefreshKind::everything())
    );
    sys.refresh_cpu_specifics(CpuRefreshKind::everything());

    unsafe {
        if FIRST_CALL {
            FIRST_CALL = false;
            sleep(Duration::from_secs(1));
            sys.refresh_cpu_specifics(CpuRefreshKind::everything());
        }

        let usage = sys.global_cpu_usage() as f64;

        let smoothed_usage = if PREV_USAGE == 0.0 { 0.0 } else { usage };
        PREV_USAGE = usage;

        Ok(smoothed_usage)
    }
}
