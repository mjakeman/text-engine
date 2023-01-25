use crate::table::Document;

mod table;
mod cursor;

fn main() {
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