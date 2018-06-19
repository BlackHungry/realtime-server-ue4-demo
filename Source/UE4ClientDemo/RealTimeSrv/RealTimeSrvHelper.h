// Fill out your copyright notice in the Description page of Project Settings.

#pragma once




#define RTS_SHOW_DEBUG_SCREEN_MSG					false
#define RTS_SHOW_DEBUG_OUTPUT_LOG					false
#define RTS_SHOW_DEBUG_OUTPUT_LOG_EXTRA				false


//Current Class Name + Function Name where this is called!
#define STR_CUR_CLASS_FUNC							(FString(__FUNCTION__))

//Current Class where this is called!
#define STR_CUR_CLASS								(FString(__FUNCTION__).Left(FString(__FUNCTION__).Find(TEXT(":"))) )

//Current Function Name where this is called!
#define STR_CUR_FUNC								(FString(__FUNCTION__).Right(FString(__FUNCTION__).Len() - FString(__FUNCTION__).Find(TEXT("::")) - 2 ))

//Current Line Number in the code where this is called!
#define STR_CUR_LINE								(FString::FromInt(__LINE__))

//Current Class and Line Number where this is called!
#define STR_CUR_CLASS_LINE							(STR_CUR_CLASS + "(" + STR_CUR_LINE + ")")

//Current Class and Line Number where this is called!
#define STR_CUR_CLASS_FUNC_LINE						(STR_CUR_CLASS_FUNC + "(" + STR_CUR_LINE + ")")

//Current Function Signature where this is called!
#define STR_CUR_FUNCSIG								(FString(__FUNCSIG__))


////////////// Screen Message
// 	Gives you the Class name and exact line number where you print a message to yourself!


#define R_MSG(TimeToDisplay )											if (RTS_SHOW_DEBUG_SCREEN_MSG) (GEngine->AddOnScreenDebugMessage(-1, (float)TimeToDisplay, FColor::Red, *(STR_CUR_CLASS_FUNC_LINE )) )

#define R_MSG_1(TimeToDisplay, StringParam1)							if (RTS_SHOW_DEBUG_SCREEN_MSG) (GEngine->AddOnScreenDebugMessage(-1, (float)TimeToDisplay, FColor::Red, *(STR_CUR_CLASS_FUNC_LINE + "  :  " + StringParam1)) )

#define R_MSG_2(TimeToDisplay, StringParam1, StringParam2)     			if (RTS_SHOW_DEBUG_SCREEN_MSG) (GEngine->AddOnScreenDebugMessage(-1, (float)TimeToDisplay, FColor::Red, *(STR_CUR_CLASS_FUNC_LINE + "  :  " + StringParam1 + "      " + StringParam2)) )

#define R_MSG_N(TimeToDisplay, StringParam1, NumericalParam2)     		if (RTS_SHOW_DEBUG_SCREEN_MSG) (GEngine->AddOnScreenDebugMessage(-1, (float)TimeToDisplay, FColor::Red, FString::Printf( TEXT("%s  :  %s    %f"), *STR_CUR_CLASS_FUNC_LINE, *FString(StringParam1), float(NumericalParam2) ) ) )

#define R_MSG_M(TimeToDisplay, FormatString, ...)     					if (RTS_SHOW_DEBUG_SCREEN_MSG) (GEngine->AddOnScreenDebugMessage(-1, (float)TimeToDisplay, FColor::Red, FString::Printf( TEXT("%s  :  %s"), *STR_CUR_CLASS_FUNC_LINE, *FString::Printf(TEXT(FormatString), ##__VA_ARGS__ ) ) ) )



///////// UE LOG!

// Example usage: R_LOG();
#define	R_LOG() 		           										if (RTS_SHOW_DEBUG_OUTPUT_LOG) UE_LOG(LogTemp, Warning, TEXT("%s"), *STR_CUR_CLASS_FUNC_LINE )

// Example usage: R_LOG_1( "Action!" );
#define R_LOG_1(StringParam1) 		           							if (RTS_SHOW_DEBUG_OUTPUT_LOG) UE_LOG(LogTemp, Warning, TEXT("%s  :  %s"), *STR_CUR_CLASS_FUNC_LINE, *FString(StringParam1))

// Example usage: R_LOG_2("Action!", "Cut!");
#define R_LOG_2(StringParam1, StringParam2) 	       					if (RTS_SHOW_DEBUG_OUTPUT_LOG) UE_LOG(LogTemp, Warning, TEXT("%s  :  %s     %s"), *STR_CUR_CLASS_FUNC_LINE, *FString(StringParam1), *FString(StringParam2))

// Example usage: R_LOG_N("Action!", 88.f);
#define R_LOG_N(StringParam1, NumericalParam2) 	       					if (RTS_SHOW_DEBUG_OUTPUT_LOG) UE_LOG(LogTemp, Warning, TEXT("%s  :  %s    %f"), *STR_CUR_CLASS_FUNC_LINE, *FString(StringParam1), float(NumericalParam2) )

// 
#define R_LOG_M(FormatString, ...)     									if (RTS_SHOW_DEBUG_OUTPUT_LOG) UE_LOG(LogTemp, Warning, TEXT("%s  :  %s"), *STR_CUR_CLASS_FUNC_LINE, *FString::Printf(TEXT(FormatString), ##__VA_ARGS__ ) )


///////// UE LOG_EXTRA!

#define	R_LOG_EXTRA() 		           									if (RTS_SHOW_DEBUG_OUTPUT_LOG_EXTRA) UE_LOG(LogTemp, Warning, TEXT("%s"), *STR_CUR_CLASS_FUNC_LINE )

#define R_LOG_1_EXTRA(StringParam1) 		           					if (RTS_SHOW_DEBUG_OUTPUT_LOG_EXTRA) UE_LOG(LogTemp, Warning, TEXT("%s  :  %s"), *STR_CUR_CLASS_FUNC_LINE, *FString(StringParam1))

#define R_LOG_2_EXTRA(StringParam1, StringParam2) 	       				if (RTS_SHOW_DEBUG_OUTPUT_LOG_EXTRA) UE_LOG(LogTemp, Warning, TEXT("%s  :  %s     %s"), *STR_CUR_CLASS_FUNC_LINE, *FString(StringParam1), *FString(StringParam2))

#define R_LOG_N_EXTRA(StringParam1, NumericalParam2) 	       			if (RTS_SHOW_DEBUG_OUTPUT_LOG_EXTRA) UE_LOG(LogTemp, Warning, TEXT("%s  :  %s    %f"), *STR_CUR_CLASS_FUNC_LINE, *FString(StringParam1), float(NumericalParam2) )

#define R_LOG_M_EXTRA(FormatString, ...)     							if (RTS_SHOW_DEBUG_OUTPUT_LOG_EXTRA) UE_LOG(LogTemp, Warning, TEXT("%s  :  %s"), *STR_CUR_CLASS_FUNC_LINE, *FString::Printf(TEXT(FormatString), ##__VA_ARGS__ ) )



// PacketSequenceNumber
typedef unsigned short													PacketSN;
#define PACKET_SEQUENCE_NUMBER_BIT_WIDE									(16)
#define MAX_PACKET_SEQUENCE_NUMBER										(65535)
#define HALF_MAX_PACKET_SEQUENCE_NUMBER									(32768)
				
typedef unsigned int													ChunkPacketID;
#define CHUNK_PACKET_ID_BIT_WIDE										(32)
#define MAX_CHUNK_PACKET_ID												(4294967296)
#define HALF_MAX_CHUNK_PACKET_ID										(2147483648)

#define MAX_PACKET_BYTE_LENGTH											(1024)

#define ACK_BIT_FIELD_BYTE_LEN											(4)



namespace RealTimeSrvHelper
{
	void ScreenMsg( const FString& Msg );
	void ScreenMsg( const FString& Msg, const FString& Msg2 );
	void ScreenMsg( const FString& Msg, const float FloatValue );


	bool SequenceGreaterThanOrEqual( PacketSN s1, PacketSN s2 );
	bool SequenceGreaterThan( PacketSN s1, PacketSN s2 );

	bool ChunkPacketIDGreaterThanOrEqual( ChunkPacketID s1, ChunkPacketID s2 );
	bool ChunkPacketIDGreaterThan( ChunkPacketID s1, ChunkPacketID s2 );
};