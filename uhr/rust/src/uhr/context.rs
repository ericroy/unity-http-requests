use error_chain::error_chain;
use tokio::runtime::Runtime;
use tokio::runtime::Builder;
use reqwest::ClientBuilder;
use reqwest::Client;

error_chain! {
    foreign_links {
        Io(std::io::Error);
        HttpRequest(reqwest::Error);
        ParseIntError(std::num::ParseIntError);
    }
}

pub struct Context {
    pub runtime: Runtime,
    pub client: Client,
}

impl Context {
    pub fn new() -> Result<Context> {
        Ok(Context {
            runtime: Builder::new_multi_thread()
                .worker_threads(4)
                .thread_name("uhr-worker")
                .thread_stack_size(3 * 1024 * 1024)
                .build()?,
            client: ClientBuilder::new()
                .build()?,
        })
    }
}
