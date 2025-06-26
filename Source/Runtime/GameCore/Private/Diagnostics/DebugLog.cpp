// Fill out your copyright notice in the Description page of Project Settings.


#include "Diagnostics/DebugLog.h"

void Log(const TCHAR* msg) {
	UE_LOG(LogTemp, Warning, TEXT("%s"), msg)
}
