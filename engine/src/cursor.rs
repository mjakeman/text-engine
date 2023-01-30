use crate::table::Document;

struct Cursor<'a> {
    document: &'a Document,
    index: usize
}

enum Amount {
    Character,
    Word,
    Sentence,
    Paragraph
}

impl Cursor<'_> {
    fn new(document: &Document) -> Cursor {
        Cursor {
            document,
            index: 0
        }
    }

    fn move_forward(&self, quantity: usize, amount: Amount) {
        match amount {
            Amount::Character => {
                let index = self.index;
                let mut travelled = 0;

                loop {
                    if let Some(run) = self.document.get_run_at_index(index) {

                        let len = self.document.get_text(run).chars().count();
                        if travelled + len > quantity {
                            // within
                        }

                    } else { break }
                }
            }
            Amount::Word => {}
            Amount::Sentence => {}
            Amount::Paragraph => {}
        }
    }
}