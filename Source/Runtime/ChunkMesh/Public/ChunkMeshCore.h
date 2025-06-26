// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Deleter that only allows objects to be deleted on the rendering thread
template <typename Type>
struct FRenderThreadDeleter
{
	void operator()(Type* object) const
	{
		if (IsInRenderingThread())
		{
			delete object;
		}
		else
		{
			ENQUEUE_RENDER_COMMAND(FRenderThreadDeleterCommand)(
				[object](FRHICommandListImmediate& rHICmdList)
				{
					delete static_cast<Type*>(object);
				}
			);
		}
	}
};