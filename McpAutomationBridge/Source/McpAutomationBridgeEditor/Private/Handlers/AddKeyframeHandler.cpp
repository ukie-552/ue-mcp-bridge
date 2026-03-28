#include "CoreMinimal.h"
#include "McpCommand.h"
#include "LevelSequence.h"
#include "MovieScene.h"
#include "MovieSceneTrack.h"
#include "MovieSceneSection.h"
#include "Tracks/MovieScene3DTransformTrack.h"
#include "Tracks/MovieSceneFloatTrack.h"
#include "Tracks/MovieSceneDoubleTrack.h"
#include "Sections/MovieScene3DTransformSection.h"
#include "Sections/MovieSceneFloatSection.h"
#include "Channels/MovieSceneChannel.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Channels/MovieSceneDoubleChannel.h"
#include "Channels/MovieSceneChannelData.h"

class FMcpAddKeyframeHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("add_keyframe"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString SequencePath;
        if (!Params->TryGetStringField(TEXT("sequence_path"), SequencePath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'sequence_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString BindingName;
        Params->TryGetStringField(TEXT("binding_name"), BindingName);

        FString TrackType;
        if (!Params->TryGetStringField(TEXT("track_type"), TrackType))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'track_type' parameter"), TEXT("MISSING_PARAM"));
        }

        double Time = 0.0;
        Params->TryGetNumberField(TEXT("time"), Time);

        int32 FrameRate = 30;
        Params->TryGetNumberField(TEXT("frame_rate"), FrameRate);

        FFrameNumber FrameNumber = FFrameNumber(static_cast<int32>(Time * FrameRate));

        ULevelSequence* Sequence = LoadObject<ULevelSequence>(nullptr, *SequencePath);
        if (!Sequence)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Level sequence not found: %s"), *SequencePath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        UMovieScene* MovieScene = Sequence->GetMovieScene();
        if (!MovieScene)
        {
            return FMcpCommandResult::Failure(TEXT("Movie scene not found"), TEXT("NO_MOVIE_SCENE"));
        }

        FGuid BindingGuid;
        if (!BindingName.IsEmpty())
        {
            for (const FMovieSceneBinding& Binding : MovieScene->GetBindings())
            {
                if (Binding.GetName() == BindingName)
                {
                    BindingGuid = Binding.GetObjectGuid();
                    break;
                }
            }
        }

        bool bSuccess = false;
        FString ChannelName;

        if (TrackType == TEXT("Transform") || TrackType == TEXT("3DTransform"))
        {
            bSuccess = AddTransformKeyframe(MovieScene, BindingGuid, FrameNumber, Params, ChannelName);
        }
        else if (TrackType == TEXT("Float"))
        {
            bSuccess = AddFloatKeyframe(MovieScene, BindingGuid, FrameNumber, Params, ChannelName);
        }
        else if (TrackType == TEXT("Double"))
        {
            bSuccess = AddDoubleKeyframe(MovieScene, BindingGuid, FrameNumber, Params, ChannelName);
        }

        if (!bSuccess)
        {
            return FMcpCommandResult::Failure(TEXT("Failed to add keyframe"), TEXT("KEYFRAME_FAILED"));
        }

        Sequence->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("added"), true);
        Result->SetNumberField(TEXT("time"), Time);
        Result->SetNumberField(TEXT("frame"), FrameNumber.Value);
        Result->SetStringField(TEXT("track_type"), TrackType);
        Result->SetStringField(TEXT("channel"), ChannelName);

        if (!BindingName.IsEmpty())
        {
            Result->SetStringField(TEXT("binding_name"), BindingName);
        }

        return FMcpCommandResult::Success(Result);
    }

private:
    bool AddTransformKeyframe(UMovieScene* MovieScene, const FGuid& BindingGuid, FFrameNumber FrameNumber, const TSharedPtr<FJsonObject>& Params, FString& OutChannelName)
    {
        if (!MovieScene || !BindingGuid.IsValid())
        {
            return false;
        }

        UMovieScene3DTransformTrack* TransformTrack = MovieScene->FindTrack<UMovieScene3DTransformTrack>(BindingGuid);
        if (!TransformTrack)
        {
            TransformTrack = MovieScene->AddTrack<UMovieScene3DTransformTrack>(BindingGuid);
        }

        if (!TransformTrack)
        {
            return false;
        }

        TArray<UMovieSceneSection*> Sections = TransformTrack->GetAllSections();
        UMovieScene3DTransformSection* TransformSection = nullptr;

        if (Sections.Num() > 0)
        {
            TransformSection = Cast<UMovieScene3DTransformSection>(Sections[0]);
        }

        if (!TransformSection)
        {
            TransformSection = Cast<UMovieScene3DTransformSection>(TransformTrack->CreateNewSection());
            TransformTrack->AddSection(*TransformSection);
        }

        if (!TransformSection)
        {
            return false;
        }

        TransformSection->SetRange(TRange<FFrameNumber>::All());

        FString Channel;
        Params->TryGetStringField(TEXT("channel"), Channel);
        OutChannelName = Channel;

        double Value = 0.0;
        Params->TryGetNumberField(TEXT("value"), Value);

        int32 ChannelIndex = 0;
        if (Channel == TEXT("LocationX") || Channel == TEXT("TranslationX")) ChannelIndex = 0;
        else if (Channel == TEXT("LocationY") || Channel == TEXT("TranslationY")) ChannelIndex = 1;
        else if (Channel == TEXT("LocationZ") || Channel == TEXT("TranslationZ")) ChannelIndex = 2;
        else if (Channel == TEXT("RotationX") || Channel == TEXT("RotationPitch")) ChannelIndex = 3;
        else if (Channel == TEXT("RotationY") || Channel == TEXT("RotationYaw")) ChannelIndex = 4;
        else if (Channel == TEXT("RotationZ") || Channel == TEXT("RotationRoll")) ChannelIndex = 5;
        else if (Channel == TEXT("ScaleX")) ChannelIndex = 6;
        else if (Channel == TEXT("ScaleY")) ChannelIndex = 7;
        else if (Channel == TEXT("ScaleZ")) ChannelIndex = 8;

        FMovieSceneDoubleChannel* DoubleChannel = TransformSection->GetChannel(ChannelIndex);
        if (DoubleChannel)
        {
            FMovieSceneChannelValueHelper<double> ValueHelper(*DoubleChannel, FrameNumber);
            ValueHelper = Value;
        }

        return true;
    }

    bool AddFloatKeyframe(UMovieScene* MovieScene, const FGuid& BindingGuid, FFrameNumber FrameNumber, const TSharedPtr<FJsonObject>& Params, FString& OutChannelName)
    {
        FString ChannelName;
        Params->TryGetStringField(TEXT("channel"), ChannelName);
        OutChannelName = ChannelName;

        double Value = 0.0;
        Params->TryGetNumberField(TEXT("value"), Value);

        UMovieSceneFloatTrack* FloatTrack = nullptr;

        if (BindingGuid.IsValid())
        {
            FloatTrack = MovieScene->FindTrack<UMovieSceneFloatTrack>(BindingGuid);
            if (!FloatTrack)
            {
                FloatTrack = MovieScene->AddTrack<UMovieSceneFloatTrack>(BindingGuid);
            }
        }
        else
        {
            FloatTrack = MovieScene->AddMasterTrack<UMovieSceneFloatTrack>();
        }

        if (!FloatTrack)
        {
            return false;
        }

        TArray<UMovieSceneSection*> Sections = FloatTrack->GetAllSections();
        UMovieSceneFloatSection* FloatSection = nullptr;

        if (Sections.Num() > 0)
        {
            FloatSection = Cast<UMovieSceneFloatSection>(Sections[0]);
        }

        if (!FloatSection)
        {
            FloatSection = Cast<UMovieSceneFloatSection>(FloatTrack->CreateNewSection());
            FloatTrack->AddSection(*FloatSection);
        }

        if (!FloatSection)
        {
            return false;
        }

        FloatSection->SetRange(TRange<FFrameNumber>::All());

        FMovieSceneFloatChannel* FloatChannel = FloatSection->GetChannel(0);
        if (FloatChannel)
        {
            FMovieSceneChannelValueHelper<float> ValueHelper(*FloatChannel, FrameNumber);
            ValueHelper = static_cast<float>(Value);
        }

        return true;
    }

    bool AddDoubleKeyframe(UMovieScene* MovieScene, const FGuid& BindingGuid, FFrameNumber FrameNumber, const TSharedPtr<FJsonObject>& Params, FString& OutChannelName)
    {
        FString ChannelName;
        Params->TryGetStringField(TEXT("channel"), ChannelName);
        OutChannelName = ChannelName;

        double Value = 0.0;
        Params->TryGetNumberField(TEXT("value"), Value);

        UMovieSceneDoubleTrack* DoubleTrack = nullptr;

        if (BindingGuid.IsValid())
        {
            DoubleTrack = MovieScene->FindTrack<UMovieSceneDoubleTrack>(BindingGuid);
            if (!DoubleTrack)
            {
                DoubleTrack = MovieScene->AddTrack<UMovieSceneDoubleTrack>(BindingGuid);
            }
        }
        else
        {
            DoubleTrack = MovieScene->AddMasterTrack<UMovieSceneDoubleTrack>();
        }

        if (!DoubleTrack)
        {
            return false;
        }

        TArray<UMovieSceneSection*> Sections = DoubleTrack->GetAllSections();
        UMovieSceneDoubleSection* DoubleSection = nullptr;

        if (Sections.Num() > 0)
        {
            DoubleSection = Cast<UMovieSceneDoubleSection>(Sections[0]);
        }

        if (!DoubleSection)
        {
            DoubleSection = Cast<UMovieSceneDoubleSection>(DoubleTrack->CreateNewSection());
            DoubleTrack->AddSection(*DoubleSection);
        }

        if (!DoubleSection)
        {
            return false;
        }

        DoubleSection->SetRange(TRange<FFrameNumber>::All());

        FMovieSceneDoubleChannel* DoubleChannel = DoubleSection->GetChannel();
        if (DoubleChannel)
        {
            FMovieSceneChannelValueHelper<double> ValueHelper(*DoubleChannel, FrameNumber);
            ValueHelper = Value;
        }

        return true;
    }
};

REGISTER_MCP_COMMAND(FMcpAddKeyframeHandler)
