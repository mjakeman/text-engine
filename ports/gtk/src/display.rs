mod imp;

use std::cell::RefCell;

use glib::Object;
use gtk::glib;
use gtk::subclass::prelude::ObjectSubclassIsExt;

use text_engine::Document;

glib::wrapper! {
    pub struct Display(ObjectSubclass<imp::Display>)
        @extends gtk::Widget,
        @implements gtk::Accessible, gtk::Buildable, gtk::ConstraintTarget;
}

impl Display {
    pub fn new() -> Self {
        Object::builder().build()
    }

    pub fn set_document(&self, document: Document) {
        let imp = self.imp();
        imp.document.replace(Some(document));
    }
}
