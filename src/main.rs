mod system_monitor;
use anyhow::Result;
use system_monitor::cpu::get_cpu_usage;
use system_monitor::gpu::get_gpu_info;
use system_monitor::ram::get_ram_usage;
use std::time::Duration;
use std::thread::sleep;

fn main() -> Result<()> {
    loop {
        let cpu_usage = get_cpu_usage()?;
        let ram_usage = get_ram_usage()?;
        let gpus = get_gpu_info()?;

        #[cfg(target_os = "windows")]
        std::process::Command::new("cmd").args(&["/C", "cls"]).status()?;
        #[cfg(not(target_os = "windows"))]
        print!("\x1B[2J\x1B[1;1H");

        println!("CPU Usage: {:.2}%", cpu_usage);
        println!("RAM Usage: {:.2} GB", ram_usage);

        for (i, gpu) in gpus.iter().enumerate() {
            println!("GPU {}: {}", i + 1, gpu.name);
            println!("  GPU Usage: {:.2}%", gpu.gpu_usage);
            println!("  Temperature: {:.2}°C", gpu.temperature);
            println!("  Total VRAM: {:.2} GB", gpu.total_vram);
            println!("  Used VRAM: {:.2} GB", gpu.used_vram);
        }
        sleep(Duration::from_secs(1));
    }
}