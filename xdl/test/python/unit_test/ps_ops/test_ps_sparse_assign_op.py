# Copyright 2018 Alibaba Group. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==============================================================================

import xdl
import unittest
import numpy as np
from xdl.python.lib.datatype import *
from xdl.python.lib.graph import execute

class TestPsSparseAssignOp(unittest.TestCase):
    def test_all(self):
        var = xdl.Variable(name="w", dtype=DataType.float, shape=[4,2], initializer=xdl.Zeros())
        execute(xdl.variable_registers())
        execute(xdl.global_initializers())
        op = xdl.ps_sparse_assign_op(
            var_name="w", var_type="index", 
            ids=np.array([1,2], dtype=np.int32),
            values=np.array([[1,2],[3,4]], dtype=np.float32))
        execute(op)
        ret = execute(var.value)
        self.assertTrue((ret == np.array([[0,0],[1,2],[3,4],[0,0]], dtype=np.float32)).all())

def suite():
    return unittest.TestLoader().loadTestsFromTestCase(TestPsSparseAssignOp)

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())
