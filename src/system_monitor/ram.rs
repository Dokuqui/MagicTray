use anyhow::Result;
use sysinfo::{RefreshKind, System, MemoryRefreshKind};

pub fn get_ram_usage() -> Result<f64> {
    let mut sys = System::new_with_specifics(
        RefreshKind::everything().with_memory(MemoryRefreshKind::default()),
    );

    sys.refresh_memory();
    let total = sys.total_memory() as f64 / (1024.0 * 1024.0 * 1024.0);
    let avail = sys.available_memory() as f64 / (1024.0 * 1024.0 * 1024.0);
    Ok(total - avail)
}
