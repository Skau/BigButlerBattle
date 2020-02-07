// Fill out your copyright notice in the Description page of Project Settings.


#include "BBBBPFLibrary.h"
#include "Misc/ConfigCacheIni.h"

FString UBBBBPFLibrary::GetProjectVersion()
{
	FString ProjectVersion;
	GConfig->GetString(
		TEXT("/Script/EngineSettings.GeneralProjectSettings"),
		TEXT("ProjectVersion"),
		ProjectVersion,
		GGameIni
	);
	return ProjectVersion;
}