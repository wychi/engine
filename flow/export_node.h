// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_EXPORT_NODE_H_
#define FLUTTER_FLOW_EXPORT_NODE_H_

#include <memory>

#include <zx/eventpair.h>

#include "dart-pkg/zircon/sdk_ext/handle.h"
#include "flutter/flow/scene_update_context.h"
#include "lib/fxl/build_config.h"
#include "lib/fxl/macros.h"
#include "lib/fxl/memory/ref_counted.h"
#include "lib/ui/scenic/client/resources.h"
#include "third_party/flutter/fml/task_runner.h"
#include "third_party/skia/include/core/SkPoint.h"

namespace flow {

// Wrapper class for ExportNode to use on UI Thread. When ExportNodeHolder is
// destroyed, a task is posted on the Rasterizer thread to dispose the resources
// held by the ExportNode.
class ExportNodeHolder : public fxl::RefCountedThreadSafe<ExportNodeHolder> {
 public:
  ExportNodeHolder(fxl::RefPtr<fxl::TaskRunner> gpu_task_runner,
                   fxl::RefPtr<zircon::dart::Handle> export_token_handle);
  ~ExportNodeHolder();

  // Calls Bind() on the wrapped ExportNode.
  void Bind(SceneUpdateContext& context,
            scenic_lib::ContainerNode& container,
            const SkPoint& offset,
            bool hit_testable);

  ExportNode* export_node() { return export_node_.get(); }

 private:
  fxl::RefPtr<fxl::TaskRunner> gpu_task_runner_;
  std::unique_ptr<ExportNode> export_node_;

  FRIEND_MAKE_REF_COUNTED(ExportNodeHolder);
  FRIEND_REF_COUNTED_THREAD_SAFE(ExportNodeHolder);
  FXL_DISALLOW_COPY_AND_ASSIGN(ExportNodeHolder);
};

// Represents a node which is being exported from the session.
// This object is created on the UI thread but the entity node it contains
// must be created and destroyed by the rasterizer thread.
class ExportNode {
 public:
  ExportNode(fxl::RefPtr<zircon::dart::Handle> export_token_handle);

  ~ExportNode();

  // Binds the export token to the entity node and adds it as a child of
  // the specified container. Must be called on the Rasterizer thread.
  void Bind(SceneUpdateContext& context,
            scenic_lib::ContainerNode& container,
            const SkPoint& offset,
            bool hit_testable);

 private:
  friend class SceneUpdateContext;
  friend class ExportNodeHolder;

  // Cleans up resources held and removes this ExportNode from
  // SceneUpdateContext. Must be called on the Rasterizer thread.
  void Dispose(bool remove_from_scene_update_context);

  // Member variables can only be read or modified on Rasterizer thread.
  SceneUpdateContext* scene_update_context_ = nullptr;
  zx::eventpair export_token_;
  std::unique_ptr<scenic_lib::EntityNode> node_;

  FXL_DISALLOW_COPY_AND_ASSIGN(ExportNode);
};

}  // namespace flow

#endif  // FLUTTER_FLOW_EXPORT_NODE_H_
