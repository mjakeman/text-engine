use std::borrow::Borrow;
use gtk::{glib, graphene};
use gtk::gdk;
use gtk::graphene::{Point, Rect};
use gtk::pango;
use gtk::pango::AttrList;
use gtk::pango::GlyphString;
use gtk::subclass::prelude::*;
use gtk::traits::{SnapshotExt, StyleContextExt};
use gtk::traits::WidgetExt;
use text_engine::{Colour, DefaultLayoutManager, FontBackend, Rectangle};
use text_engine::RenderCommand::*;

use std::cell::RefCell;
use adw::gdk::pango::{Context, ffi, Layout};
use adw::gdk::RGBA;
use adw::glib::translate::{ToGlibPtr, ToGlibPtrMut};
use gtk::pango::FontDescription;

use text_engine::Document;

// Object holding the state
#[derive(Default)]
pub struct Display {
    pub document: RefCell<Option<Document>>
}

// The central trait for subclassing a GObject
#[glib::object_subclass]
impl ObjectSubclass for Display {
    const NAME: &'static str = "TextEngineDisplay";
    type Type = super::Display;
    type ParentType = gtk::Widget;
}

// Trait shared by all GObjects
impl ObjectImpl for Display {}

// Trait shared by all widgets
impl WidgetImpl for Display {
    fn snapshot(&self, snapshot: &gtk::Snapshot) {

        let widget = self.obj();
        let color = widget.style_context().color();

        let backend = PangoBackend { context: widget.pango_context() };

        if let Some(doc) = self.document.borrow().as_ref() {
            let display_list = doc.layout::<PangoBackend>(&backend, widget.width(), &DefaultLayoutManager {});
            for command in &display_list.commands {
                match command {
                    RenderBox(rect, colour) => {
                        if let Some(colour) = colour {
                            snapshot.append_color(&colour_to_gdk_rgba(colour), &rectangle_to_graphene_rect(rect));
                        }
                    },
                    RenderText(x, y, text) => {
                        // TODO: We recreate the layout here and inside the layout engine
                        // Store this as a resource in the render layer and cache it between
                        // redraws. We only want to recompute the delta.
                        let layout = pango::Layout::new(&widget.pango_context());
                        let font_desc = FontDescription::from_string("Noto Sans");
                        layout.set_font_description(Some(&font_desc));
                        layout.set_text(text);

                        snapshot.translate(&Point::new(*x as f32, *y as f32));
                        snapshot.append_layout(&layout, &color);
                    }
                }
            }
        }
    }
}

struct PangoBackend {
    context: Context
}

impl FontBackend for PangoBackend {
    fn measure_height_for_paragraph(&self, text: &String, width: i32) -> i32 {
        let text_layout = Layout::new(&self.context);
        let font_desc = FontDescription::from_string("Noto Sans");
        text_layout.set_font_description(Some(&font_desc));
        text_layout.set_text(&text);
        text_layout.set_width(width * pango::SCALE);
        text_layout.pixel_size().1
    }
}

fn colour_to_gdk_rgba(colour: &Colour) -> RGBA {
    RGBA::new((colour.red as f32) / 255f32,
              (colour.green as f32) / 255f32,
              (colour.blue as f32) / 255f32,
              1.0)
}

fn rectangle_to_graphene_rect(rect: &Rectangle) -> graphene::Rect {
    Rect::new(rect.x as f32,
              rect.y as f32,
              rect.w as f32,
              rect.h as f32)
}