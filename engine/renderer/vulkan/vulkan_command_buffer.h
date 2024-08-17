#pragma once

enum class VulkanCommandBufferState {
	Ready,
	Recording,
	InRenderPass,
	RecordingEnded,
	Submitted,
	NotAllocated
};