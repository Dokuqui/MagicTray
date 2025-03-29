use anyhow::{Result, Context};
use nvml_wrapper::Nvml;

#[derive(Debug)]
pub struct GPUInfo {
    pub name: String,
    pub gpu_usage: f64,
    pub temperature: f64,
    pub total_vram: f64,
    pub used_vram: f64,
}

pub fn get_gpu_info() -> Result<Vec<GPUInfo>> {
    let mut gpus = Vec::new();

    // NVIDIA monitoring
    if let Ok(nvml) = Nvml::init() {
        let device_count = nvml.device_count()?;
        for i in 0..device_count {
            let device = nvml.device_by_index(i)?;
            let name = device.name()?;
            let usage = device.utilization_rates()?;
            let temp = device.temperature(nvml_wrapper::enum_wrappers::device::TemperatureSensor::Gpu)?;
            let memory = device.memory_info()?;

            gpus.push(GPUInfo {
                name,
                gpu_usage: usage.gpu as f64,
                temperature: temp as f64,
                total_vram: memory.total as f64 / (1024.0 * 1024.0 * 1024.0),
                used_vram: memory.used as f64 / (1024.0 * 1024.0 * 1024.0),
            });
        }
    }

    Ok(gpus)
}
