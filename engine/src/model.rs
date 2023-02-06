/* model.rs
 *
 * Copyright 2023 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

use crate::layout::{LayoutManager, LayoutBox, DisplayList, Rectangle};

#[derive(Clone, Copy)]
pub struct TextData {
    append: bool,
    start_index: usize,
    end_index: usize
}

enum Data {
    Text(TextData),
    Opaque
}

type NodeId = usize;

/**
 * A 'thing' in the document
 */
pub trait Element {
    // TODO: Something about cursors, runs, walking to next
    fn build_layout_tree(&self, visitor: &dyn LayoutManager) -> LayoutBox;
}

/** Block Elements **/

pub trait Block: Element {}

/**
 * Paragraph: Block element which only contains inlines.
 */
pub struct Paragraph {
    pub children: Vec<Box<dyn Inline>>
}

impl Element for Paragraph {
    fn build_layout_tree(&self, visitor: &dyn LayoutManager) -> LayoutBox {
        visitor.build_paragraph_layout_tree(self)
    }
}
impl Block for Paragraph {}

/**
 * Frame: Block element which contains other blocks
 */
pub struct Frame {
    pub children: Vec<Box<dyn Block>>
}

impl Element for Frame {
    fn build_layout_tree(&self, visitor: &dyn LayoutManager) -> LayoutBox {
        visitor.build_frame_layout_tree(self)
    }
}
impl Block for Frame {}

/**
 * Info Box: Block element which contains a frame
 */
pub struct InfoBox {
    child: Frame
}

impl Element for InfoBox {
    fn build_layout_tree(&self, visitor: &dyn LayoutManager) -> LayoutBox {
        visitor.build_info_box_layout_tree(self)
    }
}
impl Block for InfoBox {}


/** Inline Elements **/

pub trait Inline: Element {}

/**
 * Run: Inline text that is contiguously formatted
 */

pub struct Run {
    pub text: TextData,
    // format: Style
}

impl Element for Run {
    fn build_layout_tree(&self, visitor: &dyn LayoutManager) -> LayoutBox {
        visitor.build_run_layout_tree(self)
    }
}
impl Inline for Run {}

/**
 * Equation: Inline MathML equation
 */

pub struct Equation {
    
}

/*pub struct Node {
    parent_id: Option<NodeId>,
    next_id: Option<NodeId>,
    prev_id: Option<NodeId>,
    first_child_id: Option<NodeId>,
    last_child_id: Option<NodeId>,
    
    data: Data
}

struct ChildrenIter<'a> {
    tree: &'a Tree,
    current_id: Option<NodeId>
}

impl ChildrenIter<'_> {
    fn new(tree: &Tree, parent_id: NodeId) -> ChildrenIter {
        
        let current_id = tree.get(parent_id).unwrap().first_child_id;

        ChildrenIter {
            tree,
            current_id
        }
    }
}

impl Iterator for ChildrenIter<'_> {
    type Item = NodeId;

    fn next(&mut self) -> Option<Self::Item> {

        if let Some(id) = self.current_id {
            let current_item = self.tree.get(id).unwrap();
            self.current_id = current_item.next_id;
            return Some(id);
        }

        None
    }
}

pub struct Tree {
    arena: Vec<Node>,
    root: NodeId,
    unused: Vec<NodeId>
}

impl Tree {
    fn new() -> Tree {
        let node = Node {
            parent_id: None,
            next_id: None,
            prev_id: None,
            first_child_id: None,
            last_child_id: None,
            data: Data::Opaque 
        };

        let node_id = 0;

        Tree {
            arena: vec![node],
            root: node_id,
            unused: vec![]
        }
    }

    fn create_node(&mut self, parent_id: NodeId, data: Data) -> (&mut Node, NodeId) {
        let node = Node {
            parent_id: Some(parent_id),
            next_id: None,
            prev_id: None,
            first_child_id: None,
            last_child_id: None,
            data
        };

        let id = self.arena.len();

        self.arena.push(node);
        return (self.arena.get_mut(id).unwrap(), id);
    }

    fn get(&self, id: NodeId) -> Option<&Node> {
        self.arena.get(id)
    }

    fn get_mut(&mut self, id: NodeId) -> Option<&mut Node> {
        self.arena.get_mut(id)
    }

    fn get_children(&self, node: NodeId) -> ChildrenIter {
        ChildrenIter::new(&self, node)
    }

    fn append_child(&mut self, parent_id: NodeId, data: Data) -> NodeId {
        let (_, id) = self.create_node(parent_id, data);

        let parent = self.arena.get_mut(parent_id).unwrap();
        if parent.first_child_id == None {
            parent.first_child_id = Some(id);
        }
        parent.last_child_id = Some(id);

        return id;
    }

    fn append_sibling(&mut self, sibling_id: NodeId, data: Data) -> NodeId {
        let parent_id = {
            self.arena.get(sibling_id).unwrap().parent_id.unwrap()
        };

        let (node, id) = self.create_node(parent_id, data);
        node.prev_id = Some(sibling_id);

        let sibling = self.arena.get_mut(sibling_id).unwrap();
        sibling.next_id = Some(id);

        self.arena.get_mut(parent_id).unwrap().last_child_id = Some(id);

        return self.arena.len() - 1;
    }
}*/

pub struct Document {
    pub(crate) buffer : String,
    pub(crate) append : String,
    /*tree: Tree*/
    root: Frame
}

impl Document {
    pub fn new(initial: Option<&str>) -> Document {
        if let Some(initial) = initial {
            let run = Run {
                text: TextData {
                    start_index: 0,
                    end_index: initial.len(),
                    append: false
                }
            };

            let mut root = Frame {
                children: vec![ Box::new(Paragraph { children: vec![ Box::new(run) ] }) ]
            };

            return Document {
                buffer: String::from(initial),
                append: String::new(),
                root
            }
        }

        Document {
            buffer: String::new(),
            append: String::new(),
            root: Frame { children: vec![] }
        }
    }

    pub fn resolve_ref(&self, data: TextData) -> Option<String> {
        match data.append {
            true => &self.append,
            false => &self.buffer
        }.get(data.start_index..data.end_index).map(|s| s.to_string())
    }

    pub fn layout(&self, width: i32, visitor: &dyn LayoutManager) -> DisplayList {
        let layout_tree = self.root.build_layout_tree(visitor);
        let viewport = Rectangle { x: 0, y: 0, w: width, h: -1 };
        let (commands, _req_height) = layout_tree.layout(self, viewport);
        DisplayList { commands }
    }

    /*pub fn insert(&mut self, byte_index: usize, text: &str) {
        let append_buffer_index = self.append.len();
        self.append.push_str(text);

        let run_to_split = self.runs.iter_mut()
            .position(|run| run.start_index <= byte_index && run.end_index > byte_index);

        if let Some(run_index) = run_to_split {

            let (split_index, end_index, append_buffer) = {
                let run = self.runs.get(run_index).unwrap();

                let index_within_run = byte_index - run.start_index;
                let split_index = run.start_index + index_within_run;

                (split_index, run.end_index, run.append_buffer)
            };

            if let Some(run) = self.runs.get_mut(run_index) {
                run.end_index = split_index;
            }

            let mut insert_run = Run {
                start_index: append_buffer_index,
                end_index: append_buffer_index + text.len(),
                append_buffer: true
            };
            self.runs.insert(run_index + 1, insert_run);

            let mut after_run = Run {
                start_index: split_index,
                end_index,
                append_buffer: append_buffer
            };
            self.runs.insert(run_index + 2, after_run);

            return
        }

        let mut new_run = Run {
            start_index: append_buffer_index,
            end_index: append_buffer_index + text.len(),
            append_buffer: true
        };

        self.runs.push(new_run);
    }*/

    /*pub fn get_text_dfs<'a>(&self, tree: &Tree, output: &mut String) {
        for child in tree.get_children(node_id) {
            self.get_text_dfs(tree, child, output);
        }

        if let Some(text) = self.get_text(tree.get(node_id).unwrap()) {
            output.push_str(text);
        }
        
    }

    pub fn get_all_text<'a>(&self) -> String {
        let mut string_builder = String::new();
        self.get_text_dfs(&self.root, &mut string_builder);
        string_builder
    }

    pub fn get_text(&self, node: &Node) -> Option<&str> {
        if let Data::Text(text_data) = &node.data {
            let range = text_data.start_index..text_data.end_index;
            return match text_data.append {
                true => Some(&self.append[range]),
                false => Some(&self.buffer[range])
            }
        }
        None
    }*/
}

#[cfg(test)]
mod tests {
    use crate::model::Document;

    /*#[test]
    fn test_insert_at_start() {
        let mut document = Document::new(Some("Hell🌍 World"));
        document.insert(0, "Prefix: ");
        assert_eq!(document.get_all_text(), "Prefix: Hell🌍 World");
    }

    #[test]
    fn test_insert_at_end() {
        let mut document = Document::new(Some("Hell🌍 World"));
        document.insert(15, " (Suffix)");
        assert_eq!(document.get_all_text(), "Hell🌍 World (Suffix)");
    }

    #[test]
    fn test_insert_middle() {
        let mut document = Document::new(Some("Hell🌍 World"));
        document.insert(8, " 🎶🇫🇷😔");
        assert_eq!(document.get_all_text(), "Hell🌍 🎶🇫🇷😔 World");
    }*/
}