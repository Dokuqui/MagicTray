mod system_monitor;
use anyhow::Result;
use system_monitor::cpu::get_cpu_usage;
use system_monitor::ram::get_ram_usage;
use std::time::Duration;
use std::thread::sleep;

fn main() -> Result<()> {
    loop {
        let cpu_usage = get_cpu_usage()?;
        let ram_usage = get_ram_usage()?;

        #[cfg(target_os = "windows")]
        std::process::Command::new("cmd").args(&["/C", "cls"]).status()?;
        #[cfg(not(target_os = "windows"))]
        print!("\x1B[2J\x1B[1;1H");

        println!("CPU Usage: {:.2}%", cpu_usage);
        println!("RAM Usage: {:.2} GB", ram_usage);
        sleep(Duration::from_secs(1));
    }
}