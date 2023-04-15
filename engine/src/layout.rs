use std::borrow::BorrowMut;
use std::ops::Add;

use crate::{model::{Frame, Paragraph, Run, InfoBox, Element, TextData}, Document};

#[derive(Clone, Copy)]
pub struct Rectangle {
    pub x: i32,
    pub y: i32,
    pub w: i32,
    pub h: i32
}

#[derive(Clone, Copy)]
pub struct Extents {
    pub top: i32,
    pub left: i32,
    pub bottom: i32,
    pub right: i32
}

impl Rectangle {
    fn empty() -> Rectangle {
        Rectangle { x: 0, y: 0, w: 0, h: 0 }
    }

    fn with_equal_size(d: i32) -> Rectangle {
        Rectangle { x: d, y: d, w: d, h: d }
    }

    fn new(x: i32, y: i32, w: i32, h: i32) -> Rectangle {
        Rectangle { x, y, w, h }
    }
}

impl Extents {
    fn empty() -> Extents {
        Extents { top: 0, bottom: 0, left: 0, right: 0 }
    }

    fn with_equal_size(d: i32) -> Extents {
        Extents { top: d, bottom: d, left: d, right: d }
    }

    fn new(top: i32, bottom: i32, left: i32, right: i32) -> Extents {
        Extents { top, bottom, left, right }
    }
}

type InlineChildren = Vec<LayoutBox<InlineFlow>>;
type BlockChildren = Vec<LayoutBox<BlockFlow>>;

pub trait LayoutManager {
    fn build_frame_layout_tree(&self, frame: &Frame) -> LayoutBox<BlockFlow>;
    fn build_paragraph_layout_tree(&self, paragraph: &Paragraph) -> LayoutBox<BlockFlow>;
    fn build_run_layout_tree(&self, run: &Run) -> LayoutBox<InlineFlow>;
    fn build_info_box_layout_tree(&self, infobox: &InfoBox) -> LayoutBox<BlockFlow>;
}

pub struct DefaultLayoutManager {}

impl LayoutManager for DefaultLayoutManager {
    fn build_frame_layout_tree(&self, frame: &Frame) -> LayoutBox<BlockFlow> {

        let mut children: BlockChildren = vec![];

        for child in &frame.children {
            let subtree = child.build_layout_tree(self);
            children.push(subtree);
        }

        LayoutBox {
            flow: BlockFlow {
                is_anonymous: false
            },
            margins: Extents::empty(),
            padding: Extents::empty(),
            background: None,
            foreground: BLACK,
            children: LayoutChildren::with_block_children(children)
        }
    }

    fn build_paragraph_layout_tree(&self, paragraph: &Paragraph) -> LayoutBox<BlockFlow> {

        let mut children: InlineChildren = vec![];

        for child in &paragraph.children {
            let subtree = child.build_layout_tree(self);
            children.push(subtree);
        }

        let mut block = LayoutBox {
            flow: BlockFlow {
                is_anonymous: false
            },
            margins: Extents::empty(),
            padding: Extents::empty(),
            background: None,
            foreground: BLACK,
            children: LayoutChildren::with_inline_children(children)
        };

        block
    }

    fn build_run_layout_tree(&self, run: &Run) -> LayoutBox<InlineFlow> {
        let inline = LayoutBox {
            flow: InlineFlow {
                text: run.text
            },
            margins: Extents::empty(),
            padding: Extents::empty(),
            background: None,
            foreground: BLACK,
            children: LayoutChildren::None
        };

        inline
    }

    fn build_info_box_layout_tree(&self, infobox: &InfoBox) -> LayoutBox<BlockFlow> {

        let mut children: BlockChildren = vec![];

        for child in &infobox.child.children {
            let subtree = child.build_layout_tree(self);
            children.push(subtree);
        }

        let inline = LayoutBox {
            flow: BlockFlow {
                is_anonymous: false
            },
            margins: Extents::empty(),
            padding: Extents::with_equal_size(10),
            background: Some(BLUE),
            foreground: BLACK,
            children: LayoutChildren::Block(children)
        };

        inline
    }
}

pub enum FlowContext {
    Inline,
    Block,
    Anonymous
}

pub trait LayoutFlow {
    fn get_flow_context(&self) -> FlowContext;
}

pub struct BlockFlow {
    is_anonymous: bool
}

impl LayoutFlow for BlockFlow {
    fn get_flow_context(&self) -> FlowContext {
        if self.is_anonymous { FlowContext::Anonymous } else { FlowContext::Block }
    }
}

pub struct InlineFlow {
    // TODO: Make this Option<TextData> and support non-text inline
    // flows like equations and images
    text: TextData
}

impl LayoutFlow for InlineFlow {
    fn get_flow_context(&self) -> FlowContext {
        FlowContext::Inline
    }
}

enum LayoutChildren {
    Inline(InlineChildren),
    Block(BlockChildren),
    None
}

impl LayoutChildren {
    fn with_block_children(children: BlockChildren) -> LayoutChildren {
        LayoutChildren::Block(children)
    }

    fn with_inline_children(children: InlineChildren) -> LayoutChildren {
        LayoutChildren::Inline(children)
    }
}

#[derive(Copy, Clone)]
pub struct Colour {
    pub red: u32,
    pub green: u32,
    pub blue: u32,
}

pub static BLACK: Colour = Colour { red: 0, green: 0, blue: 0 };
pub static WHITE: Colour = Colour { red: 255, green: 255, blue: 255 };

pub static RED: Colour = Colour { red: 255, green: 0, blue: 0 };
pub static GREEN: Colour = Colour { red: 0, green: 255, blue: 0 };
pub static BLUE: Colour = Colour { red: 0, green: 0, blue: 255 };

pub struct LayoutBox<T: LayoutFlow> {
    flow: T,
    margins: Extents,
    padding: Extents,
    background: Option<Colour>,
    foreground: Colour,
    children: LayoutChildren
}

pub struct DisplayList {
    pub commands: Vec<RenderCommand>
}

pub enum RenderCommand {
    RenderText(i32, i32, i32, String),
    RenderBox(Rectangle, Option<Colour>)
}

pub trait FontBackend {
    fn measure_height_for_paragraph(&self, text: &String, width: i32) -> i32;
}

impl <T: LayoutFlow> LayoutBox<T> {
    pub fn layout<B: FontBackend>(&self, document: &Document, backend: &B, rect: Rectangle) -> (Vec<RenderCommand>, i32) {
        let mut commands: Vec<RenderCommand> = vec![];

        let mut content_rect = rect;

        // TODO: Current bg, border, etc
        // commands.push(RenderRectangle);
        // commands.push(RenderIcon);
        // req_height += self.margins...
        let offset_left = (self.margins.left + self.padding.left);
        let offset_top = (self.margins.top + self.padding.top);
        let offset_bottom = (self.margins.bottom + self.padding.bottom);
        let offset_right = (self.margins.right + self.padding.right);
        content_rect.w -= (offset_left + offset_right);
        content_rect.x += offset_left;
        content_rect.y += offset_top;

        // TODO: Account for margins/padding
        let (mut child_commands, mut req_height) = match &self.children {
            LayoutChildren::Inline(children) => self.layout_inline_children::<B>(document, backend, children, content_rect),
            LayoutChildren::Block(children) => self.layout_block_children::<B>(document, backend, children, content_rect),
            LayoutChildren::None => (vec![], 0)
        };

        let mut margin_box = rect;
        margin_box.y += self.margins.top;
        margin_box.x += self.margins.left;
        margin_box.w -= self.margins.right;
        margin_box.h = req_height + self.padding.top + self.padding.bottom;

        req_height += (offset_top + offset_bottom);
        commands.push(RenderCommand::RenderBox(margin_box, self.background));
        commands.append(&mut child_commands);

        (commands, req_height)
    }

    fn layout_inline_children<B: FontBackend>(&self, document: &Document, backend: &B, children: &InlineChildren, rect: Rectangle) -> (Vec<RenderCommand>, i32) {
        let mut commands = vec![];

        let mut contiguous_text = String::new();

        for child in children {
            if let Some(text) = document.resolve_ref(child.flow.text) {
                contiguous_text += &text;
            }
        }

        let height = B::measure_height_for_paragraph(backend, &contiguous_text, rect.w);
        commands.push(RenderCommand::RenderText(rect.x, rect.y, rect.w, contiguous_text));

        (commands, height)
    }

    fn layout_block_children<B: FontBackend>(&self, document: &Document, backend: &B, children: &BlockChildren, rect: Rectangle) -> (Vec<RenderCommand>, i32) {
        let mut commands = vec![];
        let mut req_height = 0;

        let mut content_rect = rect;

        for child in children {
            let (mut child_commands, child_req_height) = child.layout::<B>(document, backend, content_rect);
            commands.append(&mut child_commands);
            req_height += child_req_height;

            content_rect.y = rect.y + req_height;
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