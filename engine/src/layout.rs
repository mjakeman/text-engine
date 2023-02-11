use std::borrow::BorrowMut;
use std::ops::Add;
use pango::{Layout, Context};

use crate::{model::{Frame, Paragraph, Run, InfoBox, Element, TextData}, Document};

#[derive(Clone, Copy)]
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
            margins: Rectangle::empty(),
            padding: Rectangle::empty(),
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
            margins: Rectangle::empty(),
            padding: Rectangle::empty(),
            children: LayoutChildren::with_inline_children(children)
        };



        block
    }

    fn build_run_layout_tree(&self, run: &Run) -> LayoutBox<InlineFlow> {
        let inline = LayoutBox {
            flow: InlineFlow {
                text: run.text
            },
            margins: Rectangle::empty(),
            padding: Rectangle::empty(),
            children: LayoutChildren::None
        };

        inline
    }

    fn build_info_box_layout_tree(&self, infobox: &InfoBox) -> LayoutBox<BlockFlow> {
        todo!()
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

pub struct LayoutBox<T: LayoutFlow> {
    flow: T,
    margins: Rectangle,
    padding: Rectangle,
    children: LayoutChildren
}

pub struct DisplayList {
    pub commands: Vec<RenderCommand>
}

pub enum RenderCommand {
    RenderText(i32, i32, String),
    RenderBox(Rectangle)
}

impl <T: LayoutFlow> LayoutBox<T> {
    pub fn layout(&self, document: &Document, rect: Rectangle) -> (Vec<RenderCommand>, i32) {
        let mut req_height = 0;
        let mut commands: Vec<RenderCommand> = vec![];

        // TODO: Current bg, border, etc
        // commands.push(RenderRectangle);
        // commands.push(RenderIcon);
        // req_height += self.margins...

        // TODO: Account for margins/padding
        let (commands, req_height) = match &self.children {
            LayoutChildren::Inline(children) => self.layout_inline_children(document, children, rect),
            LayoutChildren::Block(children) => self.layout_block_children(document, children, rect),
            LayoutChildren::None => (vec![], 0)
        };

        (commands, req_height)
    }

    fn layout_inline_children(&self, document: &Document, children: &InlineChildren, rect: Rectangle) -> (Vec<RenderCommand>, i32) {
        let mut commands = vec![];

        let mut contiguous_text = String::new();

        for child in children {
            if let Some(text) = document.resolve_ref(child.flow.text) {
                contiguous_text += &text;
            }
        }

        let text_layout = Layout::new(&Context::new());
        text_layout.set_text(&contiguous_text);
        text_layout.set_width(rect.w);
        commands.push(RenderCommand::RenderText(rect.x, rect.y, contiguous_text));

        (commands, text_layout.height())
    }

    fn layout_block_children(&self, document: &Document, children: &BlockChildren, rect: Rectangle) -> (Vec<RenderCommand>, i32) {
        let mut commands = vec![];
        let mut req_height = 0;

        for child in children {
            let (mut child_commands, child_req_height) = child.layout(document, rect);
            commands.append(&mut child_commands);
            req_height += child_req_height;
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