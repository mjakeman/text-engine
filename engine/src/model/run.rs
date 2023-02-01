/* run.rs
 *
 * Copyright 2023 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

use super::fragment::Fragment;

pub struct Run {
    pub(crate) start_index : usize,
    pub(crate) end_index : usize,
    pub(crate) append_buffer : bool
}

impl Run {
    pub fn length(&self) -> usize {
        self.end_index - self.start_index
    }
}

impl Fragment for Run {}