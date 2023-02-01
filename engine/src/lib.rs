mod table;
mod cursor;

pub use crate::table::Document;

#[no_mangle]
pub extern "C" fn init() {
    let mut document = Document::new(Some("Hell🌍 World"));

    let text = document.get_all_text();
    println!("Version 0: {}", text);

    document.insert(15, ", again!");

    let text = document.get_all_text();
    println!("Version 1: {}", text);

    document.insert(23, " (no really!)");

    let text = document.get_all_text();
    println!("Version 2: {}", text);

    document.insert(8, " to the entire");

    let text = document.get_all_text();
    println!("Version 3: {}", text);
}

pub fn get_sample_document() -> Document {
    let mut document = Document::new(Some("Hell🌍 World"));
    document.insert(15, ", again!");
    document.insert(23, " (no really!)");
    document.insert(8, " to the entire");
    document
}