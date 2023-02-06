use pango::{Layout, Context};

use crate::{model::{Frame, Paragraph, Run, InfoBox, Element, TextData}, Document};

pub struct Rectangle {
    pub x: i32,
    pub y: i32,
    pub w: i32,
    pub h: i32
}

impl Rectangle {
    fn empty() -> Rectangle {
        Rectangle { x: 0, y: 0, w: 0, h: 0 }
    }

    fn new(x: i32, y: i32, w: i32, h: i32) -> Rectangle {
        Rectangle { x, y, w, h }
    }
}

pub trait LayoutManager {
    fn build_frame_layout_tree(&self, frame: &Frame) -> LayoutBox;
    fn build_paragraph_layout_tree(&self, paragraph: &Paragraph) -> LayoutBox;
    fn build_run_layout_tree(&self, run: &Run) -> LayoutBox;
    fn build_info_box_layout_tree(&self, infobox: &InfoBox) -> LayoutBox;
}

pub struct DefaultLayoutManager {}

impl LayoutManager for DefaultLayoutManager {
    fn build_frame_layout_tree(&self, frame: &Frame) -> LayoutBox {
        let mut block = LayoutBox {
            box_type: BoxType::Block,
            margins: Rectangle::empty(),
            padding: Rectangle::empty(),
            children: vec![]
        };

        for child in &frame.children {
            let subtree = child.build_layout_tree(self);
            block.children.push(subtree);
        }

        block
    }

    fn build_paragraph_layout_tree(&self, paragraph: &Paragraph) -> LayoutBox {
        let mut block = LayoutBox {
            box_type: BoxType::Block,
            margins: Rectangle::empty(),
            padding: Rectangle::empty(),
            children: vec![]
        };

        for child in &paragraph.children {
            let subtree = child.build_layout_tree(self);
            block.children.push(subtree);
        }

        block
    }

    fn build_run_layout_tree(&self, run: &Run) -> LayoutBox {
        let inline = LayoutBox {
            box_type: BoxType::Inline(Some(Text { data: run.text })),
            margins: Rectangle::empty(),
            padding: Rectangle::empty(),
            children: vec![]
        };

        inline
    }

    fn build_info_box_layout_tree(&self, infobox: &InfoBox) -> LayoutBox {
        todo!()
    }
}

pub struct LayoutBox {
    box_type: BoxType,
    margins: Rectangle,
    padding: Rectangle,
    children: Vec<LayoutBox>
}

pub struct DisplayList {
    pub commands: Vec<RenderCommand>
}

pub enum RenderCommand {
    RenderText(i32, i32, String),
    RenderBox(Rectangle)
}

impl LayoutBox {
    pub fn layout(&self, document: &Document, rect: Rectangle) -> (Vec<RenderCommand>, i32) {
        let mut req_height = 0;

        // TODO: Current bg, border, etc
        let mut commands: Vec<RenderCommand> = vec![];

        match &self.box_type {
            BoxType::Block => {
                for child in &self.children {
                    let viewport = Rectangle { x: rect.x, y: rect.y + req_height, w: rect.w, h: -1 };
                    let (mut child_commands, height) = child.layout(&document, viewport);
                    req_height += height;
                    commands.append(&mut child_commands);
                }
                commands.push(RenderCommand::RenderBox(Rectangle { x: rect.x, y: rect.y, w: rect.w, h: req_height }))
            },
            BoxType::Inline(text) => {
                if let Some(text) = text {
                    // TODO: proper text layout
                    let text_layout = Layout::new(&Context::new());
                    if let Some(text) = document.resolve_ref(text.data) {
                        text_layout.set_text(&text);
                        text_layout.set_width(rect.w);
                        req_height += text_layout.height();
                        commands.push(RenderCommand::RenderText(rect.x, rect.y, text))
                    }
                }
                
            }
        }

        (commands, req_height)
    }
}

struct Text {
    data: TextData,
}

enum BoxType {
    Inline(Option<Text>),
    Block
}