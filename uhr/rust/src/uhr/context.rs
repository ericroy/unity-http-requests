use std::collections::HashMap;
use std::future::Future;
use error_chain::error_chain;

error_chain! {
    foreign_links {
        Io(std::io::Error);
        HttpRequest(reqwest::Error);
        ParseIntError(std::num::ParseIntError);
    }
}

pub type RequestID = u32;

pub struct ResultStorage {

}

pub struct Context {
    pub runtime: tokio::runtime::Runtime,
    pub client: reqwest::Client,
    pub results: HashMap<RequestID, Box<dyn Future<Output = reqwest::Result<reqwest::Response>>>>,
    pub results_storage: HashMap<RequestID, ResultStorage>,
    pub next_request_id: u32,
}

impl Context {
    pub fn new() -> Result<Context> {
        Ok(Context {
            runtime: tokio::runtime::Builder::new_multi_thread()
                .worker_threads(4)
                .thread_name("uhr-worker")
                .thread_stack_size(3 * 1024 * 1024)
                .build()?,
            client: reqwest::ClientBuilder::new()
                .build()?,
            results: HashMap::with_capacity(16),
            results_storage: HashMap::with_capacity(16),
            next_request_id: 1,
        })
    }


}
