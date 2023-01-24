use crate::table::{Document, get_all_text, get_text, insert, Run};

mod table;

fn main() {
    let mut document = Document::new(Some("Hello World"));

    let text = get_all_text(&document);
    println!("Version 0: {}", text);

    insert(&mut document, 11, ", again!");

    let text = get_all_text(&document);
    println!("Version 1: {}", text);

    insert(&mut document, 19, " (no really!)");

    let text = get_all_text(&document);
    println!("Version 2: {}", text);
}
