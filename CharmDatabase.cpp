#include "stdafx.h"
#include "CharmDatabase.h"
#include "Solution.h"
#include "Armor.h"
#include "Skill.h"
#include <fstream>
#include <cmath>

using namespace System;

void GetCharms( List_t< Charm^ >^ list, Query^ query, const unsigned max_slots )
{
	for each( Skill^ skill in query->skills )
	{
		unsigned mx[ 4 ];
		for( unsigned slots = max_slots + 1; slots --> 0; )
		{
			unsigned slot_max = 0;
			for( unsigned typ = 0; typ < max_slots; ++typ )
			{
				Map_t< Ability^, unsigned >^ _map = CharmDatabase::table[ typ, slots ];
				if( _map->ContainsKey( skill->ability ) )
				{
					const unsigned curr_max = _map[ skill->ability ];
					slot_max = slot_max > curr_max ? slot_max : curr_max;
				}
			}
			mx[ slots ] = slot_max;
			if( max_slots > 0 && slots == 0 && slot_max <= mx[ 1 ] )
				continue;
			//single skill charm
			Charm^ ct = gcnew Charm;
			ct->num_slots = slots;
			if( slot_max )
				ct->abilities.Add( gcnew AbilityPair( skill->ability, slot_max ) );
			list->Add( ct );
		}
	}
}

List_t< Charm^ >^ CharmDatabase::GetCharms( Query^ query, const bool use_two_skill_charms )
{
	List_t< Charm^ >^ res = gcnew List_t< Charm^ >;
	bool have[ 4 ] = { false, false, false, false };

	if( use_two_skill_charms )
	{
		//two skill charms
		for( int i = 0; i < query->skills.Count; ++i )
		{
			for( int j = 0; j < i; ++j )
			{
				List_t< Charm^ >^ to_add = table2[ query->skills[ i ]->ability ][ query->skills[ j ]->ability ];
				res->AddRange( to_add );
			}
		}
	}
	else
	{
		//one skill charms
		::GetCharms( res, query, 1 + ( query->hr >= 5 ) + ( query->hr >= 4 ) );
	}

	//no skill charms
	for each( Charm^ charm in res )
	{
		have[ charm->num_slots ] = true;
	}
	
	if( query->hr >= 5 )
	{
		if( !have[ 3 ] )
			res->Add( gcnew Charm( 3 ) );
	}
	else if( query->hr >= 4 )
	{
		if( !have[ 2 ] )
			res->Add( gcnew Charm( 2 ) );
	}
	else
	{
		if( !have[ 1 ] )
			res->Add( gcnew Charm( 1 ) );
	}
	return res;
}

#define CUSTOM_TXT L"Data/mycharms.txt"

void CharmDatabase::SaveCustom()
{
	//slots,skill1,num,skill2,num
	IO::StreamWriter fout( CUSTOM_TXT );
	fout.WriteLine( L"#Format: NumSlots,Skill1,Points1,Skill2,Points2" );
	for each( Charm^ ch in mycharms )
	{
		fout.Write( Convert::ToString( ch->num_slots ) );
		for( int i = 0; i < 2; ++i )
		{
			if( i < ch->abilities.Count )
				fout.Write( L"," + ch->abilities[ i ]->ability->name + L"," + Convert::ToString( ch->abilities[ i ]->amount ) );
			else fout.Write( L",," );
		}
		fout.WriteLine();
	}
}

void CharmDatabase::LoadCustom()
{
	mycharms.Clear();
	IO::StreamReader fin( CUSTOM_TXT );
	String^ temp;
	bool cheats = false;
	while( !fin.EndOfStream )
	{
		temp = fin.ReadLine();
		if( temp == L"" || temp[ 0 ] == L'#' ) continue;
		List_t< String^ > split;
		Utility::SplitString( %split, temp, L',' );
		if( split.Count != 5 )
		{
			Windows::Forms::MessageBox::Show( nullptr, L"Incorrect number of commas", temp );
			continue;
		}
		//slots,skill1,num,skill2,num
		Charm^ charm = gcnew Charm();
		charm->num_slots = Convert::ToUInt32( split[ 0 ] );
		
		try
		{
			if( split[ 1 ] != L"" )
			{
				Ability^ ab = Ability::FindAbility( split[ 1 ] );
				if( !ab )
				{
					Windows::Forms::MessageBox::Show( nullptr, L"\"" + split[ 1 ] + L"\": No such skill", temp );
					continue;
				}
				charm->abilities.Add( gcnew AbilityPair( ab, Convert::ToInt32( split[ 2 ] ) ) );
			}
			if( split[ 3 ] != L"" )
			{
				Ability^ ab = Ability::FindAbility( split[ 3 ] );
				if( !ab )
				{
					Windows::Forms::MessageBox::Show( nullptr, L"\"" + split[ 3 ] + L"\": No such skill", temp );
					continue;
				}
				charm->abilities.Add( gcnew AbilityPair( ab, Convert::ToInt32( split[ 4 ] ) ) );
			}
		}
		catch( Exception^ )
		{
			Windows::Forms::MessageBox::Show( nullptr, L"Could not read skill points", temp );
			continue;
		}
		
		if( CharmExists( charm ) ||
			!CharmIsIllegal( charm ) )
		{
			charm->custom = true;
			mycharms.Add( charm );
		}
		else cheats = true;
	}
	if( cheats )
		System::Windows::Forms::MessageBox::Show( StaticString( Cheater ) );
}

ref struct OmaSkill
{
	static int NAME = 0;
	static int MIN = 1;
	static int MAX = 2;

	static int FURU1 = 0;
	static int FURU2 = 1;
	static int HIKA1 = 2;
	static int HIKA2 = 3;
	static int NAZO1 = 4;

	static array< array< array< int >^ >^ >^ FURUSLO =
	{ 
		{ 
			{ 99 }, 
			{ 21, 71, 91, 99 }, 
			{ 40, 75, 93, 99 }, 
			{ 48, 78, 94, 99 }, 
			{ 56, 81, 95, 99 }, 
			{ 64, 84, 96, 99 }, 
			{ 69, 87, 97, 99 }, 
			{ 75, 90, 98, 99 }, 
			{ 81, 93, 98, 99 }, 
			{ 86, 96, 98, 99 }, 
			{ 90, 98, 99 }, 
			{ 93, 98, 99 }, 
			{ 95, 98, 99 }, 
			{ 98, 99 }, 
			{ 98, 99 }, 
			{ 98, 99 }, 
			{ 99 }, 
			{ 99 }, 
			{ 99 }, 
			{ 99 }, 
			{ 99 }
		}, 
		{ 
			{ 99 }, 
			{ 49, 84, 99 }, 
			{ 57, 87, 99 }, 
			{ 64, 89, 99 }, 
			{ 71, 91, 99 }, 
			{ 78, 94, 99 }, 
			{ 82, 96, 99 }, 
			{ 85, 97, 99 }, 
			{ 88, 98, 99 }, 
			{ 90, 98, 99 }, 
			{ 94, 99 }, 
			{ 96, 99 }, 
			{ 98, 99 }, 
			{ 98, 99 }, 
			{ 99 }, 
			{ 99 }, 
			{ 99 }, 
			{ 99 }, 
			{ 99 }, 
			{ 99 }, 
			{ 99 }
		}, 
		{ 
			{ 99 }, 
			{ 69, 99 }, 
			{ 74, 99 }, 
			{ 79, 99 }, 
			{ 84, 99 }, 
			{ 89, 99 }, 
			{ 91, 99 }, 
			{ 94, 99 }, 
			{ 98, 99 }, 
			{ 99 }, 
			{ 99 }
		}
	};

	static array< array< array< int >^ >^ >^ SKILL =
	{ 
		{ 
			{ 35, 0, 5 }, 	//聴覚保護
			{ 46, 0, 5 }, 	//風圧
			{ 27, 0, 6 }, 	//特殊攻撃
			{ 28, 0, 5 }, 	//属性攻撃
			{ 87, 0, 7 }, 	//火属性攻撃
			{ 88, 0, 7 }, 	//水属性攻撃
			{ 89, 0, 7 }, 	//雷属性攻撃
			{ 90, 0, 7 }, 	//氷属性攻撃
			{ 91, 0, 7 }, 	//龍属性攻撃
			{ 94, 0, 5 }, 	//痛撃
			{ 95, 0, 5 }, 	//重撃
			{ 59, 0, 6 }, 	//装填数
			{ 70, 0, 5 }, 	//溜め短縮
			{ 84, 0, 6 }, 	//本気
			{ 96, 0, 6 }, 	//KO
			{ 97, 0, 6 }, 	//減気攻撃
			{ 56, 0, 6 }, 	//底力
			{ 72, 0, 5 }, 	//属性耐性
			{ 34, 0, 7 }, 	//加護
			{ 52, 0, 6 }, 	//回復量
			{ 55, 0, 6 }, 	//回避性能
			{ 64, 0, 6 }, 	//回避距離
			{ 98, 0, 6 }, 	//納刀
			{ 61, 0, 5 }, 	//食事
			{ 58, 0, 5 }, 	//スタミナ
			{ 67, 0, 6 }, 	//体術
			{ 99, 0, 6 }, 	//気力回復
			{ 50, 0, 5 }, 	//運気
			{ 68, 0, 5 }, 	//捕獲
			{ 62, 0, 5 }, 	//剥ぎ取り
			{ 10, 0, 5 }, 	//匠
			{ 11, 0, 5 }, 	//剣術
			{ 9, 0, 5 }, 	//斬れ味
			{ 65, 0, 5 }, 	//抜刀会心
			{ 71, 0, 5 }, 	//抜刀減気
			{ 14, 0, 6 }, 	//ガード性能
			{ 15, 0, 6 }, 	//ガード強化
			{ 19, 0, 5 }, 	//通常弾強化
			{ 20, 0, 5 }, 	//貫通弾強化
			{ 21, 0, 5 }, 	//散弾強化
			{ 74, 0, 5 }, 	//速射
			{ 17, 0, 6 }, 	//装填速度
			{ 18, 0, 5 }	//反動
		}, 
		{ 
			{ 4, -10, 13 }, //気絶
			{ 35, -3, 3 }, 	//聴覚保護
			{ 46, -10, 3 }, //風圧
			{ 32, -10, 10 },//攻撃
			{ 12, -10, 10 },//達人
			{ 27, -10, 4 }, //特殊攻撃
			{ 28, -3, 3 }, 	//属性攻撃
			{ 87, -10, 9 }, //火属性攻撃
			{ 88, -10, 9 }, //水属性攻撃
			{ 89, -10, 9 }, //雷属性攻撃
			{ 90, -10, 9 }, //氷属性攻撃
			{ 91, -10, 9 }, //龍属性攻撃
			{ 94, -3, 3 }, 	//痛撃
			{ 95, -3, 3 }, 	//重撃
			{ 59, -3, 3 }, 	//装填数
			{ 70, -3, 3 }, 	//溜め短縮
			{ 84, -3, 4 }, 	//本気
			{ 96, -3, 4 }, 	//KO
			{ 97, -3, 4 }, 	//減気攻撃
			{ 56, -3, 4 }, 	//底力
			{ 29, -10, 10 },//爆弾強化
			{ 82, -10, 10 },//笛
			{ 83, -10, 10 },//砲術
			{ 33, -10, 10 },//防御
			{ 72, -3, 3 }, 	//属性耐性
			{ 39, -10, 13 },//火耐性
			{ 40, -10, 13 },//水耐性
			{ 41, -10, 13 },//雷耐性
			{ 42, -10, 13 },//氷耐性
			{ 43, -10, 13 },//龍耐性
			{ 34, -10, 9 }, //加護
			{ 7, -10, 13 }, //体力
			{ 8, -10, 12 }, //回復速度
			{ 52, -3, 4 }, 	//回復量
			{ 55, -3, 4 }, 	//回避性能
			{ 64, -3, 4 }, 	//回避距離
			{ 98, -10, 4 }, //納刀
			{ 61, -3, 3 }, 	//食事
			{ 58, -3, 3 }, 	//スタミナ
			{ 67, -3, 4 }, 	//体術
			{ 99, -3, 4 }, 	//気力回復
			{ 30, -10, 10 },//腹減り
			{ 50, -3, 3 }, 	//運気
			{ 68, -3, 3 }, 	//捕獲
			{ 62, -3, 3 }, 	//剥ぎ取り
			{ 37, -10, 12 },//広域
			{ 51, -10, 12 },//千里眼
			{ 10, -3, 3 }, 	//匠
			{ 11, -3, 3 }, 	//剣術
			{ 9, -3, 3 }, 	//斬れ味
			{ 65, -3, 3 }, 	//抜刀会心
			{ 71, -3, 3 }, 	//抜刀減気
			{ 14, -3, 4 }, 	//ガード性能
			{ 15, -3, 4 }, 	//ガード強化
			{ 13, -10, 8 }, //研ぎ師
			{ 19, -3, 3 }, 	//通常弾強化
			{ 20, -3, 3 }, 	//貫通弾強化
			{ 21, -3, 3 }, 	//散弾強化
			{ 74, -3, 3 }, 	//速射
			{ 17, -3, 4 }, 	//装填速度
			{ 18, -3, 3 }	//反動
		}, 
		{ 
			{ 4, 0, 8 }, 	//気絶
			{ 2, 0, 5 }, 	//麻痺
			{ 3, 0, 5 }, 	//睡眠
			{ 1, 0, 5 }, 	//毒
			{ 86, 0, 7 }, 	//対防御DOWN
			{ 5, 0, 7 }, 	//耐泥耐雪
			{ 76, 0, 7 }, 	//抗菌
			{ 63, 0, 7 }, 	//耐震
			{ 46, 0, 4 }, 	//風圧
			{ 32, 0, 4 }, 	//攻撃
			{ 12, 0, 4 }, 	//達人
			{ 27, 0, 4 }, 	//特殊攻撃
			{ 87, 0, 7 }, 	//火属性攻撃
			{ 88, 0, 7 }, 	//水属性攻撃
			{ 89, 0, 7 }, 	//雷属性攻撃
			{ 90, 0, 7 }, 	//氷属性攻撃
			{ 91, 0, 7 }, 	//龍属性攻撃
			{ 96, 0, 6 }, 	//KO
			{ 97, 0, 6 }, 	//減気攻撃
			{ 56, 0, 6 }, 	//底力
			{ 29, 0, 6 }, 	//爆弾強化
			{ 82, 0, 6 }, 	//笛
			{ 83, 0, 6 }, 	//砲術
			{ 33, 0, 4 }, 	//防御
			{ 39, 0, 6 }, 	//火耐性
			{ 40, 0, 6 }, 	//水耐性
			{ 41, 0, 6 }, 	//雷耐性
			{ 42, 0, 6 }, 	//氷耐性
			{ 43, 0, 6 }, 	//龍耐性
			{ 34, 0, 7 }, 	//加護
			{ 7, 0, 8 }, 	//体力
			{ 8, 0, 7 }, 	//回復速度
			{ 52, 0, 6 }, 	//回復量
			{ 55, 0, 6 }, 	//回避性能
			{ 64, 0, 6 }, 	//回避距離
			{ 98, 0, 6 }, 	//納刀
			{ 66, 0, 8 }, 	//高速設置
			{ 61, 0, 4 }, 	//食事
			{ 67, 0, 6 }, 	//体術
			{ 99, 0, 6 }, 	//気力回復
			{ 30, 0, 8 }, 	//腹減り
			{ 38, 0, 8 }, 	//運搬
			{ 37, 0, 8 }, 	//広域
			{ 51, 0, 8 }, 	//千里眼
			{ 14, 0, 6 }, 	//ガード性能
			{ 15, 0, 4 }, 	//ガード強化
			{ 13, 0, 4 }, 	//研ぎ師
			{ 22, 0, 6 }, 	//通常弾追加
			{ 23, 0, 6 }, 	//貫通弾追加
			{ 24, 0, 6 }, 	//散弾追加
			{ 25, 0, 6 }, 	//榴弾追加
			{ 26, 0, 6 }, 	//拡散弾追加
			{ 92, 0, 6 }, 	//斬裂弾追加
			{ 17, 0, 4 }, 	//装填速度
			{ 60, 0, 6 } 	//精密射撃
		}, 
		{ 
			{ 4, -10, 10 }, //気絶
			{ 2, -10, 7 }, 	//麻痺
			{ 3, -10, 7 }, 	//睡眠
			{ 1, -10, 7 }, 	//毒
			{ 86, -10, 8 }, //対防御DOWN
			{ 36, -10, 10 },//盗み無効
			{ 5, -10, 8 }, 	//耐泥耐雪
			{ 76, -10, 8 }, //抗菌
			{ 63, -10, 8 }, //耐震
			{ 44, -10, 10 },//耐暑
			{ 45, -10, 10 },//耐寒
			{ 32, -7, 7 }, 	//攻撃
			{ 12, -7, 7 }, 	//達人
			{ 27, -4, 4 }, 	//特殊攻撃
			{ 29, -8, 8 }, 	//爆弾強化
			{ 82, -8, 8 }, 	//笛
			{ 83, -8, 8 }, 	//砲術
			{ 33, -7, 7 }, 	//防御
			{ 39, -10, 10 },//火耐性
			{ 40, -10, 10 },//水耐性
			{ 41, -10, 10 },//雷耐性
			{ 42, -10, 10 },//氷耐性
			{ 43, -10, 10 },//龍耐性
			{ 7, -10, 10 }, //体力
			{ 8, -4, 4 }, 	//回復速度
			{ 66, -10, 10 },//高速設置
			{ 30, -10, 10 },//腹減り
			{ 31, -10, 13 },//食いしん坊
			{ 38, -10, 10 },//運搬
			{ 47, -10, 13 },//採取
			{ 48, -10, 10 },//高速収集
			{ 49, -10, 13 },//気まぐれ
			{ 37, -10, 10 },//広域
			{ 51, -10, 10 },//千里眼
			{ 53, -10, 13 },//調合成功率
			{ 54, -10, 10 },//調合数
			{ 57, -10, 10 },//効果持続
			{ 69, -10, 10 },//観察眼
			{ 75, -10, 10 },//こやし
			{ 85, -10, 10 },//狩人
			{ 6, -10, 10 }, //気配
			{ 22, -8, 8 }, 	//通常弾追加
			{ 23, -10, 10 },//貫通弾追加
			{ 24, -10, 10 },//散弾追加
			{ 25, -10, 10 },//榴弾追加
			{ 26, -10, 10 },//拡散弾追加
			{ 92, -10, 10 },//斬裂弾追加
			{ 60, -10, 10 },//精密射撃
			{ 78, -10, 10 },//麻痺瓶追加
			{ 79, -10, 10 },//睡眠瓶追加
			{ 77, -10, 10 },//毒瓶追加
			{ 80, -10, 10 },//強撃瓶追加
			{ 81, -10, 10 },//接撃瓶追加
			{ 93, -10, 10 }	//減気瓶追加
		}, 
		{ 
			{ 4, 0, 8 }, 	//気絶
			{ 2, 0, 5 }, 	//麻痺
			{ 3, 0, 5 }, 	//睡眠
			{ 1, 0, 5 }, 	//毒
			{ 86, 0, 7 }, 	//対防御DOWN
			{ 36, 0, 10 }, 	//盗み無効
			{ 5, 0, 7 }, 	//耐泥耐雪
			{ 76, 0, 7 }, 	//抗菌
			{ 63, 0, 7 }, 	//耐震
			{ 44, 0, 10 }, 	//耐暑
			{ 45, 0, 10 }, 	//耐寒
			{ 32, 0, 4 }, 	//攻撃
			{ 12, 0, 4 }, 	//達人
			{ 29, 0, 6 }, 	//爆弾強化
			{ 82, 0, 6 }, 	//笛
			{ 83, 0, 6 }, 	//砲術
			{ 33, 0, 4 }, 	//防御
			{ 39, 0, 6 }, 	//火耐性
			{ 40, 0, 6 }, 	//水耐性
			{ 41, 0, 6 }, 	//雷耐性
			{ 42, 0, 6 }, 	//氷耐性
			{ 43, 0, 6 }, 	//龍耐性
			{ 7, 0, 8 }, 	//体力
			{ 66, 0, 8 }, 	//高速設置
			{ 30, 0, 8 }, 	//腹減り
			{ 31, 0, 10 }, 	//食いしん坊
			{ 38, 0, 8 }, 	//運搬
			{ 47, 0, 10 }, 	//採取
			{ 48, 0, 8 }, 	//高速収集
			{ 49, 0, 10 }, 	//気まぐれ
			{ 37, 0, 8 }, 	//広域
			{ 51, 0, 8 }, 	//千里眼
			{ 53, 0, 10 }, 	//調合成功率
			{ 54, 0, 8 }, 	//調合数
			{ 57, 0, 8 }, 	//効果持続
			{ 69, 0, 8 }, 	//観察眼
			{ 75, 0, 8 }, 	//こやし
			{ 85, 0, 8 }, 	//狩人
			{ 6, 0, 8 }, 	//気配
			{ 22, 0, 6 }, 	//通常弾追加
			{ 23, 0, 6 }, 	//貫通弾追加
			{ 24, 0, 6 }, 	//散弾追加
			{ 25, 0, 6 }, 	//榴弾追加
			{ 26, 0, 6 }, 	//拡散弾追加
			{ 92, 0, 6 }, 	//斬裂弾追加
			{ 60, 0, 6 }, 	//精密射撃
			{ 78, 0, 7 }, 	//麻痺瓶追加
			{ 79, 0, 8 }, 	//睡眠瓶追加
			{ 77, 0, 8 }, 	//毒瓶追加
			{ 80, 0, 7 }, 	//強撃瓶追加
			{ 81, 0, 8 }, 	//接撃瓶追加
			{ 93, 0, 8 }	//減気瓶追加
		}
	};
};


ref class Omaget
{
	array< array< int >^ >^ SKILL1;
	array< array< int >^ >^ SKILL2;
	const int omakbn;
public:
	unsigned table_number;
	String^ charm_string;

	Omaget( const int _omakbn, String^ _charm_string )
		: omakbn( _omakbn ), charm_string( _charm_string )
	{
		if( omakbn == 0 )
		{
			SKILL1 = OmaSkill::SKILL[OmaSkill::FURU1];
			SKILL2 = OmaSkill::SKILL[OmaSkill::FURU2];
		} else if( omakbn == 1 )
		{
			SKILL1 = OmaSkill::SKILL[OmaSkill::HIKA1];
			SKILL2 = OmaSkill::SKILL[OmaSkill::HIKA2];
		} else
		{
			SKILL1 = OmaSkill::SKILL[OmaSkill::NAZO1];
			SKILL2 = nullptr;
		}
	}

	void SimulateCharm( int n )
	{
		n = rnd(n);

		int skill1 = getSkill1(n);

		n = rnd(n);

		int point1 = getPoint1(n, skill1);

		n = rnd(n);

		int skill2 = -9999;
		int point2 = -9999;
		if ((isSkill2(n)) && (SKILL2 != nullptr))
		{
			n = rnd(n);

			skill2 = getSkill2(n);

			n = rnd(n);

			array< int >^ maxmin = getPoint2MaxMin(n, skill2);

			n = rnd(n);

			point2 = getPoint2(n, maxmin);
		}

		n = rnd(n);

		int slot = getSlot(n, skill1, point1, skill2, point2);

		UpdateTable(skill1, point1, skill2, point2, slot);
	}
/*
	void FindCharm( int n, TableResult^ result, Ability^ ability1, const unsigned points1, Ability^ ability2, const unsigned points2, const unsigned num_slots, const unsigned language )
	{
		//copy-pasted code, do not modify
			n = rnd(n);
			int skill1 = getSkill1(n);
			n = rnd(n);
			int point1 = getPoint1(n, skill1);
			n = rnd(n);
			int skill2 = -9999;
			int point2 = -9999;
			if ((isSkill2(n)) && (SKILL2 != nullptr))
			{
				n = rnd(n);
				skill2 = getSkill2(n);
				n = rnd(n);
				array< int >^ maxmin = getPoint2MaxMin(n, skill2);
				n = rnd(n);
				point2 = getPoint2(n, maxmin);
			}
			n = rnd(n);
			unsigned slot = getSlot(n, skill1, point1, skill2, point2);
		//
		Ability^ ab1 = Ability::ordered_abilities[ SKILL1[skill1][OmaSkill::NAME] ];
		Ability^ ab2 = nullptr;
		if( skill2 >= 0 && SKILL1[skill1][OmaSkill::NAME] != SKILL2[skill2][OmaSkill::NAME] && point2 > 0 )
		{
			ab2 = Ability::ordered_abilities[ SKILL2[skill2][OmaSkill::NAME] ];
		}
		if( ( ability1 == nullptr || ability1 == ab1 && point1 >= (int)points1 || ability1 == ab2 && point2 >= (int)points1 ) &&
			( ability2 == nullptr || ability2 == ab2 && point2 >= (int)points2 || ability2 == ab1 && point1 >= (int)points2 ) &&
			slot >= num_slots )
		{
			if( omakbn == 0 )
				result->timeworn++;
			else if( omakbn == 1 )
				result->shining++;
			else
			{
				Assert( omakbn == 2, L"Bad charm type" );
				result->mystery++;
			}
			if( charm_string != L"Many" )
			{
				Charm temp;
				temp.num_slots = slot;
				if( ab1 )
				{
					temp.abilities.Add( gcnew AbilityPair( ab1, point1 ) );
					if( ab2 )
						temp.abilities.Add( gcnew AbilityPair( ab2, point2 ) );
				}
				if( charm_string == nullptr )
				{
					charm_string = temp.GetName( language );
				}
				else if( charm_string != temp.GetName( language ) )
				{
					charm_string = L"Many";
				}
			}
		}
	}*/
/*
	bool IsCharm( int n, Charm^ charm )
	{
		//copy-pasted code, do not modify
		n = rnd(n);
		int skill1 = getSkill1(n);
		n = rnd(n);
		int point1 = getPoint1(n, skill1);
		n = rnd(n);
		int skill2 = -9999;
		int point2 = -9999;
		if ((isSkill2(n)) && (SKILL2 != nullptr))
		{
			n = rnd(n);
			skill2 = getSkill2(n);
			n = rnd(n);
			array< int >^ maxmin = getPoint2MaxMin(n, skill2);
			n = rnd(n);
			point2 = getPoint2(n, maxmin);
		}
		n = rnd(n);
		unsigned slot = getSlot(n, skill1, point1, skill2, point2);
		//
		Ability^ ab1 = Ability::FindAbility( OmaSkill::OMAKEI_ORDER[ SKILL1[skill1][OmaSkill::NAME] ] );
		Ability^ ab2 = nullptr;
		if( skill2 >= 0 && SKILL1[skill1][OmaSkill::NAME] != SKILL2[skill2][OmaSkill::NAME] && point2 > 0 )
		{
			ab2 = Ability::FindAbility( OmaSkill::OMAKEI_ORDER[ SKILL2[skill2][OmaSkill::NAME] ] );
		}

		if( charm->abilities.Count == 0 )
			return slot == charm->num_slots;
		else if( charm->abilities.Count == 1 )
			return slot == charm->num_slots && 
			( ab1 == charm->abilities[ 0 ]->ability && point1 >= charm->abilities[ 0 ]->amount ||
			  ab2 == charm->abilities[ 0 ]->ability && point2 >= charm->abilities[ 0 ]->amount );
		else if( charm->abilities.Count == 2 )
			return slot == charm->num_slots &&
				( ab1 == charm->abilities[ 0 ]->ability && point1 == charm->abilities[ 0 ]->amount && ab2 == charm->abilities[ 1 ]->ability && point2 == charm->abilities[ 1 ]->amount ||
				  ab2 == charm->abilities[ 0 ]->ability && point2 == charm->abilities[ 0 ]->amount && ab1 == charm->abilities[ 1 ]->ability && point1 == charm->abilities[ 1 ]->amount );
		else assert( false );
		return false;
	}
*/
	static int rnd( const int n )
	{
		Assert( n < 65536 && n >= 0, L"Bad RND" );
		if( n == 0 ) return 176;

		int x = n * 16 - n * 4 - n;
		long long y = x * 16;
		long long z = y * 11367737L / 4294967296L;
		long long m = y - ( ( y - z ) / 2L + z ) / 32768L * 65363L;
		return (int)( m & 0xFFFF );
	}

	int getSkill1(int n)
	{
		return n % SKILL1->Length;
	}

	int getPoint1(int n, int skill1)
	{
		int max = SKILL1[skill1][OmaSkill::MAX];

		return n % max + 1;
	}

	bool isSkill2(int n)
	{
		int m = rndChange(n);

		switch (omakbn) {
	case 0:
		return m >= 25;
	case 1:
		return m >= 35;
		}
		return false;
	}

	int getSkill2(int n)
	{
		return n % SKILL2->Length;
	}

	array< int >^ getPoint2MaxMin( const int n, const int skill2)
	{
		const int max = SKILL2[skill2][OmaSkill::MAX];
		const int min = SKILL2[skill2][OmaSkill::MIN];

		if ((n & 0x1) == 1)
		{
			array< int >^ ret = { 1, max };
			return ret;
		}
		else
		{
			array< int >^ ret = { min, 0 };
			return ret;
		}
	}

	static int getPoint2( const int n, array< int >^ maxmin)
	{
		int min = maxmin[0];
		int max = maxmin[1];
		return n % (max - min + 1) + min;
	}

	int getSlot(int n, int skill1, int point1, int skill2, int point2)
	{
		int nn = rndChange(n);
		int p = getRankPoint(skill1, point1, skill2, point2);

		array< int >^ tbl = OmaSkill::FURUSLO[omakbn][p];
		for (int i = 0; i < tbl->Length; i++) {
			if (nn <= tbl[i]) return i;
		}
		return 0;
	}

	int getRankPoint(int skill1, int point1, int skill2, int point2)
	{
		int max1 = SKILL1[skill1][OmaSkill::MAX];
		float w1 = max1 / 10.0F;
		float px = point1 / w1;
		if ((skill2 >= 0) && (point2 > 0) && (SKILL1[skill1][OmaSkill::NAME] != SKILL2[skill2][OmaSkill::NAME]))
		{
			int max2 = SKILL2[skill2][OmaSkill::MAX];
			float w2 = max2 / 10.0F;
			px += point2 / w2;
		}
		return int( px );//int( floor( px + 0.5f ) );
	}

	static int rndChange( long long n )
	{
		long long a = n / 100;
		a *= 100;
		return (int)(n - a);
	}

	void ReportAbilityScore( Ability^ ability, const unsigned points, const unsigned slots )
	{
		Assert( points < 14, L"Skill points too high" );
		Map_t< Ability^, unsigned >^ _map = CharmDatabase::table[2-omakbn, slots ];
		Assert( _map, L"Failed to find result map" );
		if( _map->ContainsKey( ability ) )
		{
			const unsigned limit = _map[ ability ];
			_map[ ability ] = limit > points ? limit : points;
		}
		else
		{
			_map->Add( ability, points );
		}
	}

	void ReportAbilityScore( Ability^ ability1, const int points1,
							 Ability^ ability2, const int points2, const unsigned slots )
	{
		List_t< Charm^ >^ list = CharmDatabase::table2[ ability1 ][ ability2 ];
		Assert( list == CharmDatabase::table2[ ability2 ][ ability1 ], L"Charm table not symmetrical" );
		for( int i = 0; i < list->Count; ++i )
		{
			Charm^ c = list[ i ];
			Assert( c->abilities.Count == 2, L"Not enough abilities on this charm" );
			Assert( c->abilities[ 0 ]->ability == ability1 && c->abilities[ 1 ]->ability == ability2 ||
					c->abilities[ 1 ]->ability == ability1 && c->abilities[ 0 ]->ability == ability2, L"Bad abilities" );
			const bool end1 = c->abilities[ 0 ]->ability != ability1, end2 = !end1;
			if( slots > c->num_slots )
			{
				if( c->abilities[ end1 ]->amount <= points1 &&
					c->abilities[ end2 ]->amount <= points2 )
				{
					list->RemoveAt( i-- );
				}
			}
			else if( points1 > c->abilities[ end1 ]->amount )
			{
				if( c->num_slots <= slots &&
					c->abilities[ end2 ]->amount <= points2 )
				{
					list->RemoveAt( i-- );
				}
			}
			else if( points2 > c->abilities[ end2 ]->amount )
			{
				if( c->num_slots <= slots &&
					c->abilities[ end1 ]->amount <= points1 )
				{
					list->RemoveAt( i-- );
				}
			}
			else return;
		}
		Charm^ nc = gcnew Charm( slots );
		nc->abilities.Add( gcnew AbilityPair( ability1, points1 ) );
		nc->abilities.Add( gcnew AbilityPair( ability2, points2 ) );
		list->Add( nc );
	}

	void UpdateTable( int skill1, int point1, int skill2, int point2, int slot )
	{
		//Ability^ ab1 = Ability::FindAbility( OmaSkill::OMAKEI_ORDER[ SKILL1[skill1][OmaSkill::NAME] ] );
		Ability^ ab1 = Ability::ordered_abilities[ SKILL1[skill1][OmaSkill::NAME] ];
		Ability^ ab2 = nullptr;
		ReportAbilityScore( ab1, point1, slot );
		if( skill2 >= 0 && SKILL1[skill1][OmaSkill::NAME] != SKILL2[skill2][OmaSkill::NAME] ) //was != 0
		{
			//ab2 = Ability::FindAbility( OmaSkill::OMAKEI_ORDER[ SKILL2[skill2][OmaSkill::NAME] ] );
			ab2 = Ability::ordered_abilities[ SKILL2[skill2][0] ];
			if( point2 > 0 )
			{
				ReportAbilityScore( ab2, point2, slot );
				ReportAbilityScore( ab1, point1, ab2, point2, slot );
			}
		}
		const unsigned charm_hash = Charm::HashFunction( slot, ab1, point1, ab2, point2 );
		CharmDatabase::charm_hashes.Add( charm_hash );
		const unsigned ab1_index = ab1 ? ab1->static_index : Ability::static_abilities.Count;
		CharmDatabase::table_hashes[ table_number, 2-omakbn, slot ]->Add( charm_hash );
		/*
		const int p = getRankPoint(skill1, point1, skill2, point2) + slot * 2;
		res->thingo = ((omakbn == 0) && (p >= 13)) ? 2 : p >= 8 ? 1 : 0;
		return res;*/
	}
};

void RemoveDuplicates( List_t< unsigned >% list )
{
	if( list.Count < 2 )
		return;
	//assumes list is sorted
	int i = 1, j = 1;
	unsigned last = list[ 0 ];
	while( j < list.Count )
	{
		if( last < list[ j ] )
		{
			last = list[ j ];
			list[ i++ ] = list[ j++ ];
		}
		else ++j;
	}
	if( i < list.Count )
		list.RemoveRange( i, list.Count - i );
}

void CharmDatabase::GenerateCharmTable()
{
	charm_hashes.Clear();
	//setup table2
	for( int i = 1; i < Ability::static_abilities.Count; ++i )
	{
		for( int j = 0; j < i; ++j )
		{
			Ability^ ab1 = Ability::static_abilities[ i ];
			Ability^ ab2 = Ability::static_abilities[ j ];
			if( !table2.ContainsKey( ab1 ) )
				table2.Add( ab1, gcnew Map_t< Ability^, List_t< Charm^ >^ > );
			if( !table2.ContainsKey( ab2 ) )
				table2.Add( ab2, gcnew Map_t< Ability^, List_t< Charm^ >^ > );
			if( !table2[ ab1 ]->ContainsKey( ab2 ) )
			{
				Assert( !table2[ ab2 ]->ContainsKey( ab1 ), L"Ability not new in table?" );
				List_t< Charm^ >^ list = gcnew List_t< Charm^ >;
				table2[ ab1 ]->Add( ab2, list );
				table2[ ab2 ]->Add( ab1, list );
			}
			else Assert( table2[ ab2 ]->ContainsKey( ab1 ), L"Ability not in table?" );
		}
	}
	//setup table
	const int tableinit[ 12 ] = { 2, 5, 9, 26, 6, 4, 13, 18, 15, 1, 12, 3 };
	table = gcnew array< Map_t< Ability^, unsigned >^, 2 >( 3, 4 );
	table_hashes = gcnew array< List_t< unsigned >^, 3 >( 12, 3, 4 );
	for( int i = 0; i < table_hashes->GetLength( 0 ); ++i )
		for( int j = 0; j < table_hashes->GetLength( 1 ); ++j )
			for( int k = 0; k < table_hashes->GetLength( 2 ); ++k )
				table_hashes->SetValue( gcnew List_t< unsigned >, i, j, k );

	//iterate through all possible charms
	for( unsigned charm_type = 0; charm_type < 3; charm_type++ )
	{
		for( unsigned slots = 0; slots < 4; ++slots )
			table[ charm_type, slots ] = gcnew Map_t< Ability^, unsigned >;
		Omaget o( 2 - charm_type, nullptr );
		for( unsigned i = 0; i < 12; ++i )
		{
			o.table_number = i;
			int n = tableinit[ i ];
			do
			{
				o.SimulateCharm( n );
			}
			while( ( n = Omaget::rnd( n ) ) != tableinit[ i ] );
		}
	}
	for each( Ability^ ab in Ability::static_abilities )
	{
		if( ab->auto_guard )
		{
			CharmDatabase::charm_hashes.Add( Charm::HashFunction( 0, ab, 10, nullptr, 0 ) );
			break;
		}
	}
	CharmDatabase::charm_hashes.Sort();
	RemoveDuplicates( CharmDatabase::charm_hashes );
}
/*
List_t< TableResult^ >^ FindCharm( Ability^ ability1, const unsigned points1, Ability^ ability2, const int points2, const unsigned num_slots )
{
	List_t< TableResult^ >^ results = gcnew List_t< TableResult^ >;
	for( int i = 0; i < 12; ++i )
	{
		TableResult^ res = gcnew TableResult;
		res->mystery = res->shining = res->timeworn = 0;
		res->charm = nullptr;
		results->Add( res );
	}

	const int tableinit[ 12 ] = { 2, 5, 9, 26, 6, 4, 13, 18, 15, 1, 12, 3 };
	//iterate through all possible charms
	String^ charm_string = nullptr;
	for( unsigned charm_type = 0; charm_type < 3; charm_type++ )
	{
		Omaget o( 2 - charm_type, charm_string );
		for( unsigned i = 0; i < 12; ++i )
		{
			int n = tableinit[ i ];
			do
			{
				o.FindCharm( n, results[ i ], ability1, points1, ability2, points2, num_slots, language );
			}
			while( ( n = Omaget::rnd( n ) ) != tableinit[ i ] );
		}
		charm_string = o.charm_string;
	}
	if( charm_string && charm_string != L"Many" )
	{
		TableResult^ res = gcnew TableResult;
		res->charm = charm_string;
		results->Add( res );
	}

	return results;
}*/

bool CharmExists( Charm^ charm )
{
	return CharmDatabase::charm_hashes.BinarySearch( charm->GetHash() ) >= 0;
}

CharmLocationData^ FindCharm2( Ability^ ability1, const unsigned points1, Ability^ ability2, const int points2, const unsigned num_slots )
{
	const unsigned NumTables = CharmDatabase::table_hashes->GetLength( 0 );
	const unsigned NumCharmTypes = CharmDatabase::table_hashes->GetLength( 1 );
	CharmLocationData^ res = gcnew CharmLocationData();
	res->table = gcnew array< unsigned, 2 >( NumTables, NumCharmTypes );
#define UnhashCharm( X ) \
	const unsigned charm_slots = X & 0x3; \
	const unsigned charm_ability1_index = ( X >> 11 ) & 0x7F; \
	const unsigned charm_ability2_index = ( X >> 18 ) & 0x7F; \
	const unsigned charm_points1 = ( X >> 2 ) & 0xF; \
	const int charm_points2 = ( ( X >> 6 ) & 0x1F ) - 12
	if( ability1 )
	{
		unsigned count = 0, last_charm = 0;
		if( ability2 )
		{
			for( unsigned table = 0; table < NumTables; ++table )
			{
				for( unsigned charm_type = 0; charm_type < NumCharmTypes; ++charm_type )
				{
					for each( const unsigned hashed_charm in CharmDatabase::table_hashes[ table, charm_type, num_slots ] )
					{
						UnhashCharm( hashed_charm );
						if(	/*charm_slots >= num_slots &&*/
							( charm_ability1_index == ability1->static_index && charm_points1 >= points1 && charm_ability2_index == ability2->static_index && charm_points2 >= points2 ||
							  charm_ability1_index == ability2->static_index && (int)charm_points1 >= points2 && charm_ability2_index == ability1->static_index && charm_points2 >= (int)points1 ) )
						{
							res->table[ table, charm_type ]++;
							count++;
							last_charm = hashed_charm;
						}
					}
				}
			}
		}
		else
		{
			if( num_slots == 0 && ability1->name == L"" && points1 == 10 )
				return res; //Handle special case for Auto-Guard charm

			for( unsigned table = 0; table < NumTables; ++table )
			{
				for( unsigned charm_type = 0; charm_type < NumCharmTypes; ++charm_type )
				{
					for each( const unsigned hashed_charm in CharmDatabase::table_hashes[ table, charm_type, num_slots ] )
					{
						UnhashCharm( hashed_charm );
						if(	/*charm_slots >= num_slots &&*/
							( charm_ability1_index == ability1->static_index && charm_points1 >= points1 ||
							  charm_ability2_index == ability1->static_index && charm_points2 >= (int)points1 ) )
						{
							res->table[ table, charm_type ]++;
							count++;
							last_charm = hashed_charm;
						}
					}
				}
			}
		}
		if( count == 1 )
		{
			UnhashCharm( last_charm );
			res->example = gcnew Charm( charm_slots );
			if( charm_ability1_index != Ability::static_abilities.Count )
				res->example->abilities.Add( gcnew AbilityPair( Ability::static_abilities[ charm_ability1_index ], charm_points1 ) );
			if( charm_ability2_index != Ability::static_abilities.Count )
				res->example->abilities.Add( gcnew AbilityPair( Ability::static_abilities[ charm_ability2_index ], charm_points2 ) );
		}
	}
	else
	{
		for( unsigned table = 0; table < NumTables; ++table )
		{
			for( unsigned charm_type = 0; charm_type < NumCharmTypes; ++charm_type )
			{
				res->table[ table, charm_type ] = CharmDatabase::table_hashes[ table, charm_type, num_slots ]->Count;
				/*
				for each( const unsigned hashed_charm in CharmDatabase::table_hashes[ table, charm_type, num_slots, Ability::static_abilities.Count ] )
				{
					const unsigned charm_slots = hashed_charm & 0x3;
					if(	charm_slots >= num_slots )
					{
						res[ table, charm_type ]++;
						count++;
						last_charm = hashed_charm;
					}
				}*/
			}
		}
	}
	return res;

}

bool CharmIsIllegal( Charm^ charm )
{
	//return true;
	const unsigned NumTables = CharmDatabase::table_hashes->GetLength( 0 );
	const unsigned NumCharmTypes = CharmDatabase::table_hashes->GetLength( 1 );

	if( charm->abilities.Count > 0 )
	{
		Ability^ ability1 = charm->abilities[ 0 ]->ability;
		const unsigned points1 = charm->abilities[ 0 ]->amount;
		if( charm->abilities.Count == 2 )
		{
			Ability^ ability2 = charm->abilities[ 1 ]->ability;
			const int points2 = charm->abilities[ 1 ]->amount;
			for( unsigned table = 0; table < NumTables; ++table )
			{
				for( unsigned charm_type = 0; charm_type < NumCharmTypes; ++charm_type )
				{
					for each( const unsigned hashed_charm in CharmDatabase::table_hashes[ table, charm_type, charm->num_slots ] )
					{
						UnhashCharm( hashed_charm );
						if(	/*charm_slots >= num_slots &&*/
							( charm_ability1_index == ability1->static_index && charm_points1 >= points1 && charm_ability2_index == ability2->static_index && charm_points2 >= points2 ||
							charm_ability1_index == ability2->static_index && (int)charm_points1 >= points2 && charm_ability2_index == ability1->static_index && charm_points2 >= (int)points1 ) )
						{
							return false;
						}
					}
				}
			}
		}
		else
		{
			for( unsigned table = 0; table < NumTables; ++table )
			{
				for( unsigned charm_type = 0; charm_type < NumCharmTypes; ++charm_type )
				{
					for each( const unsigned hashed_charm in CharmDatabase::table_hashes[ table, charm_type, charm->num_slots ] )
					{
						UnhashCharm( hashed_charm );
						if(	/*charm_slots >= num_slots &&*/
							( charm_ability1_index == ability1->static_index && charm_points1 >= points1 ||
							charm_ability2_index == ability1->static_index && charm_points2 >= (int)points1 ) )
						{
							return false;
						}
					}
				}
			}
		}
	}
	else
	{
		for( unsigned table = 0; table < NumTables; ++table )
		{
			for( unsigned charm_type = 0; charm_type < NumCharmTypes; ++charm_type )
			{
				if( CharmDatabase::table_hashes[ table, charm_type, charm->num_slots ]->Count > 0 )
					return false;
			}
		}
	}
	return true;
}

#undef UnhashCharm
