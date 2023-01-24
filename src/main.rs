use crate::table::{Document, get_all_text, insert};

mod table;

fn main() {
    let mut document = Document::new(Some("Hell🌍 World"));

    let text = get_all_text(&document);
    println!("Version 0: {}", text);

    insert(&mut document, 15, ", again!");

    let text = get_all_text(&document);
    println!("Version 1: {}", text);

    insert(&mut document, 23, " (no really!)");

    let text = get_all_text(&document);
    println!("Version 2: {}", text);

    insert(&mut document, 8, " to the entire");

    let text = get_all_text(&document);
    println!("Version 3: {}", text);
}
