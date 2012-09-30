#ifdef JP
static char *spell_tips[VALID_REALM][32] =
{
	{
		"近くの邪悪なモンスターを感知する。",
		"怪我と体力を少し回復させる。",
		"一定時間、命中率とACにボーナスを得る。",
		"恐怖を取り除く。",
		"光源が照らしている範囲か部屋全体を永久に明るくする。",
		"近くの全ての罠と扉と階段を感知する。",
		"怪我と体力を中程度回復させる。",
		"満腹にする。",

		"アイテムにかかった弱い呪いを解除する。",
		"体内の毒を取り除く。",
		"体力を大幅に回復させ、負傷と朦朧状態も全快する。",
		"一定時間、透明なものが見えるようになる。",
		"聖なる力をもつ宝珠を放つ。邪悪なモンスターに対して大きなダメージを与えるが、善良なモンスターには効果がない。",
		"邪悪なモンスターの攻撃を防ぐバリアを張る。",
		"極めて強力な回復呪文で、負傷と朦朧状態も全快する。",
		"自分のいる床の上に、モンスターが通り抜けたり召喚されたりすることができなくなるルーンを描く。",

		"視界内の全ての悪魔にダメージを与える。",
		"アイテムにかかった強力な呪いを解除する。",
		"視界内の全てのアンデッド及び悪魔にダメージを与える。",
		"視界内の全てのモンスターを魅了する。抵抗されると無効。",
		"視界内の全ての邪悪なモンスターにダメージを与える。",
		"視界内の全ての邪悪なモンスターをテレポートさせる。抵抗されると無効。",
		"視界内の邪悪な存在に大きなダメージを与え、体力を回復し、毒、恐怖、朦朧状態、負傷から全快する。",
		"自分のいる床と周囲8マスの床の上に、モンスターが通り抜けたり召喚されたりすることができなくなるルーンを描く。",

		"一定時間、ヒーロー気分になる。",
		"長時間、命中率とACにボーナスを得る。",
		"自分の周囲にいるアンデッドを現在の階から消し去る。抵抗されると無効。",
		"すべてのステータスと経験値を回復する。",
		"最強の治癒の魔法で、負傷と朦朧状態も全快する。",
		"アイテムの持つ能力を完全に知る。",
		"隣接するモンスターに聖なるダメージを与え、視界内のモンスターにダメージ、減速、朦朧、混乱、恐怖、眠りを与える。さらに天使を召喚し、体力を回復し、恐怖を取り除き、一定時間ヒーロー気分になり、加速する。",
		"一定時間、あらゆる耐性を付け、ACと魔法防御能力を上昇させる。"
	},

	{
		"近くの全ての見えるモンスターを感知する。",
		"近距離のテレポートをする。",
		"近くの全ての扉と罠を感知する。",
		"光源が照らしている範囲か部屋全体を永久に明るくする。",
		"モンスター1体を混乱させる。抵抗されると無効。",
		"遠距離のテレポートをする。",
		"モンスター1体を眠らせる。抵抗されると無効。",
		"杖/魔法棒の充填回数を増やすか、充填中のロッドの充填時間を減らす。",

		"周辺の地形を感知する。",
		"アイテムを識別する。",
		"モンスター1体を減速さる。抵抗されると無効。",
		"視界内の全てのモンスターを眠らせる。抵抗されると無効。",
		"モンスター1体をテレポートさせる。抵抗されると無効。",
		"一定時間、加速する。",
		"近くの全てのモンスター、罠、扉、階段、財宝、そしてアイテムを感知する。",
		"アイテムの持つ能力を完全に知る。",

		"近くの全てのアイテムと財宝を感知する。",
		"モンスター1体を魅了する。抵抗されると無効。",
		"一定時間、テレパシー能力を得る。",
		"街へ移動する。地上にいるときしか使えない。",
		"現在の自分の状態を完全に知る。",
		"瞬時に上か下の階にテレポートする。",
		"地上にいるときはダンジョンの最深階へ、ダンジョンにいるときは地上へと移動する。",
		"自分のいる床の上に、モンスターが通ると爆発してダメージを与えるルーンを描く。",

		"モンスターの残り体力、最大体力、スピード、正体を知る。",
		"アイテムを自分の足元へ移動させる。",
		"その階全体を永久に照らし、ダンジョン内すべてのアイテムを感知する。さらに、一定時間テレパシー能力を得る。",
		"視界内の全てのモンスターを魅了する。抵抗されると無効。",
		"短距離内の指定した場所にテレポートする。",
		"アイテム1つをお金に変える。",
		"視界内の全てのモンスターをテレポートさせる。抵抗されると無効。",
		"一定時間、ダメージを受けなくなるバリアを張る。切れた瞬間に少しターンを消費するので注意。"
	},

	{
		"近くの全ての見えるモンスターを感知する。",
		"電撃の短いビームを放つ。",
		"近くの全ての罠と扉を感知する。",
		"満腹になる。",
		"光源が照らしている範囲か部屋全体を永久に明るくする。",
		"動物1体を魅了する。抵抗されると無効。",
		"一定時間、冷気、炎、電撃に対する耐性を得る。装備による耐性に累積する。",
		"怪我を全快させ、毒を体から完全に取り除き、体力を少し回復させる。",

		"壁を溶かして床にする。",
		"冷気のボルトもしくはビームを放つ。",
		"周辺の地形を感知し、近くの罠、扉、階段、全てのモンスターを感知する。",
		"火炎のボルトもしくはビームを放つ。",
		"光線を放つ。光りを嫌うモンスターに効果がある。",
		"視界内の全てのモンスターを減速させる。抵抗されると無効。",
		"動物を1体召喚する。",
		"体力を大幅に回復させ、負傷、朦朧状態、毒から全快する。",

		"自分のいる位置に階段を作る。",
		"一定時間、ACを上昇させる。",
		"一定時間、酸、電撃、炎、冷気、毒に対する耐性を得る。装備による耐性に累積する。",
		"周囲に木を作り出す。",
		"視界内の全ての動物を魅了する。抵抗されると無効。",
		"アイテムの持つ能力を完全に知る。",
		"自分の周囲に花崗岩の壁を作る。",
		"アイテムを酸で傷つかないよう加工する。",

		"周囲のダンジョンを揺らし、壁と床をランダムに入れ変える。",
		"全方向に向かって攻撃する。",
		"巨大な冷気の球を放つ。",
		"巨大な電撃の球を放つ。",
		"巨大な水の球を放つ。",
		"自分を中心とした光の球を発生させる。さらに、その階全体を永久に照らし、ダンジョン内すべてのアイテムを感知する。",
		"武器に炎か冷気の属性をつける。",
		"近くの全てのモンスターにダメージを与え、地震を起こし、自分を中心とした分解の球を発生させる。"
	},

	{
		"弱い魔法の矢を放つ。",
		"隣接する罠と扉を破壊する。",
		"光源が照らしている範囲か部屋全体を永久に明るくする。",
		"相手を混乱させる攻撃をできるようにする。",
		"魔法の球を放つ。",
		"炎のボルトもしくはビームを放つ。",
		"ごく小さな分解の球を放つ。",
		"遠距離のテレポートをする。",

		"モンスターにランダムな効果を与える。",
		"カオスのボルトもしくはビームを放つ。",
		"自分を中心とした轟音の球を発生させる。",
		"純粋な魔力のビームを放つ。",
		"炎の球を放つ。",
		"モンスター1体をテレポートさせる。抵抗されると無効。",
		"周辺のアイテム、モンスター、地形を破壊する。",
		"巨大なカオスの球を放つ。",

		"モンスター1体を変身させる。抵抗されると無効。",
		"全方向に対して電撃のビームを放つ。",
		"杖/魔法棒の充填回数を増やすか、充填中のロッドの充填時間を減らす。",
		"巨大な分解の球を放つ。",
		"現在の階を再構成する。",
		"ロケットを発射する。",
		"武器にカオスの属性をつける。",
		"悪魔を1体召喚する。",

		"重力のビームを放つ。",
		"自分の周辺に隕石を落とす。",
		"自分を中心とした超巨大な炎の球を発生させる。",
		"ランダムな属性の球やビームを発生させる。",
		"自分を変身させようとする。",
		"非常に強力で巨大な純粋な魔力の球を放つ。",
		"非常に強力なカオスの球を放つ。",
		"自分に周囲に向かって、ロケット、純粋な魔力の球、放射性廃棄物の球を放つ。ただし、壁に隣接して使用すると広範囲を破壊する。"
	},

	{
		"近くの生命のないモンスターを感知する。",
		"ごく小さな邪悪な力を持つボールを放つ。善良なモンスターには大きなダメージを与える。",
		"近くの邪悪なモンスターを感知する。",
		"毒の球を放つ。",
		"1体のモンスターを眠らせる。抵抗されると無効。",
		"一定時間、毒への耐性を得る。装備による耐性に累積する。",
		"モンスター1体を恐怖させ、朦朧させる。抵抗されると無効。",
		"アンデッド1体を魅了する。抵抗されると無効。",

		"生命のある者に効果のある球を放つ。",
		"地獄のボルトもしくはビームを放つ。",
		"自分を中心とした毒の球を発生させる。",
		"モンスター1体を消し去る。経験値やアイテムは手に入らない。抵抗されると無効。",
		"武器に毒の属性をつける。",
		"モンスター1体から生命力を吸いとる。吸いとった生命力によって満腹度が上がる。",
		"周囲の死体や骨を生き返す。",
		"指定した文字のモンスターを現在の階から消し去る。抵抗されると無効。",

		"狂戦士化し、恐怖を除去する。",
		"ランダムで様々な効果が起こる。",
		"暗黒のボルトもしくはビームを放つ。",
		"狂戦士化し、恐怖を除去し、加速する。",
		"武器に吸血の属性をつける。",
		"モンスター1体から生命力を吸いとる。吸いとった生命力によって体力が回復する。",
		"視界内の生命のあるモンスターにダメージを与える。",
		"巨大な暗黒の球を放つ。",

		"死の光線を放つ。",
		"1体のアンデッドを召喚する。",
		"アイテムを1つ識別する。レベルが高いとアイテムの能力を完全に知ることができる。",
		"一定時間、吸血鬼に変化する。変化している間は本来の種族の能力を失い、代わりに吸血鬼としての能力を得る。",
		"失った経験値を回復する。",
		"自分の周囲にいるモンスターを現在の階から消し去る。抵抗されると無効。",
		"邪悪な力を持つ宝珠を放つ。善良なモンスターには大きなダメージを与える。",
		"一定時間、壁を通り抜けることができ受けるダメージが軽減される幽体の状態に変身する。"
	},

	{
		"近距離のテレポートをする。",
		"1体の蜘蛛を召喚する。",
		"カードの占いをする。",
		"最深階を変更する。",
		"遠距離のテレポートをする。",
		"一定時間、テレパシー能力を得る。",
		"モンスター1体をテレポートさせる。抵抗されると無効。",
		"1体の動物を召喚する。",

		"アイテムを自分の足元へ移動させる。",
		"複数の爆発するモンスターを召喚する。",
		"1体の幽霊を召喚する。",
		"モンスター1体を加速させる。",
		"瞬時に上か下の階にテレポートする。",
		"短距離内の指定した場所にテレポートする。",
		"地上にいるときはダンジョンの最深階へ、ダンジョンにいるときは地上へと移動する。",
		"視界内の全てのモンスターをテレポートさせる。抵抗されると無効。",

		"1体のモンスターと位置を交換する。",
		"1体のアンデッドを召喚する。",
		"1体のヒドラを召喚する。",
		"複数のモンスターを召喚する。",
		"1グループのハウンドを召喚する。",
		"武器にトランプの属性をつける。",
		"ランダムにテレポートする突然変異か、自分の意思でテレポートする突然変異が身につく。",
		"1体のサイバーデーモンを召喚する。",

		"近くの全てのモンスター、罠、扉、階段、財宝、そしてアイテムを感知する。",
		"アイテムの持つ能力を完全に知る。",
		"モンスター1体の体力を回復させる。",
		"1体のドラゴンを召喚する。",
		"自分の周辺に隕石を落とす。",
		"1体の悪魔を召喚する。",
		"1体の上級アンデッドを召喚する。",
		"1体の古代ドラゴンを召喚する。"
	},

	{
		"電撃のボルトもしくはビームを放つ。",
		"扉に鍵をかける。",
		"近くの透明なモンスターを感知する。",
		"近くの全ての見えるモンスターを感知する。",
		"近距離のテレポートをする。",
		"光源が照らしている範囲か部屋全体を永久に明るくする。",
		"一直線上の全ての罠と扉を破壊する。",
		"怪我と体力を少し回復させる。",

		"近くの全ての罠と扉と階段を感知する。",
		"光源に燃料を補給する。",
		"近くの財宝を感知する。",
		"近くの魔法がかかったアイテムを感知する。",
		"近くの全てのアイテムを感知する。",
		"毒を体内から完全に取り除く。",
		"一定時間、冷気への耐性を得る。装備による耐性に累積する。",
		"一定時間、炎への耐性を得る。装備による耐性に累積する。",

		"一定時間、電撃への耐性を得る。装備による耐性に累積する。",
		"一定時間、酸への耐性を得る。装備による耐性に累積する。",
		"怪我と体力を中程度回復させる。",
		"遠距離のテレポートをする。",
		"アイテムを識別する。",
		"壁を溶かして床にする。",
		"光線を放つ。光りを嫌うモンスターに効果がある。",
		"満腹にする。",
		"一定時間、透明なものが見えるようになる。",

		"1体のエレメンタルを召喚する。",
		"瞬時に上か下の階にテレポートする。",
		"モンスター1体をテレポートさせる。抵抗されると無効。",
		"炎、電撃、冷気、酸のどれかの球を放つ。",
		"近くの全てのモンスター、罠、扉、階段、財宝、そしてアイテムを感知する。",
		"地上にいるときはダンジョンの最深階へ、ダンジョンにいるときは地上へと移動する。",
		"その階全体を永久に照らし、ダンジョン内すべてのアイテムを感知する。さらに、一定時間テレパシー能力を得る。"
	},

	{
		"一定時間、赤外線視力が増強される。",
		"一定時間、回復力が増強される。",
		"満腹になる。",
		"一定時間、冷気への耐性を得る。装備による耐性に累積する。",
		"一定時間、炎への耐性を得る。装備による耐性に累積する。",
		"一定時間、ヒーロー気分になる。",
		"一定時間、電撃への耐性を得る。装備による耐性に累積する。",
		"一定時間、酸への耐性を得る。装備による耐性に累積する。",

		"一定時間、透明なものが見えるようになる。",
		"アイテムにかかった弱い呪いを解除する。",
		"一定時間、毒への耐性を得る。装備による耐性に累積する。",
		"狂戦士化し、恐怖を除去する。",
		"現在の自分の状態を完全に知る。",
		"邪悪なモンスターの攻撃を防ぐバリアを張る。",
		"毒、朦朧状態、負傷を全快させ、幻覚を直す。",
		"一定時間、武器に冷気、炎、電撃、酸、毒のいずれかの属性をつける。",

		"一定時間、テレパシー能力を得る。",
		"一定時間、ACを上昇させる。",
		"一定時間、酸、電撃、炎、冷気、毒に対する耐性を得る。装備による耐性に累積する。",
		"一定時間、加速する。",
		"一定時間、半物質化し壁を通り抜けられるようになる。",
		"盾に反射の属性をつける。",
		"1体のゴーレムを製造する。",
		"一定時間、魔法防御力とACが上がり、混乱と盲目の耐性、反射能力、麻痺知らず、浮遊を得る。",

		"武器・防具にかけられたあらゆる魔力を完全に解除する。",
		"アイテムにかかった強力な呪いを解除する。",
		"アイテムの持つ能力を完全に知る。",
		"武器の命中率修正とダメージ修正を強化する。",
		"鎧の防御修正を強化する。",
		"武器にランダムに属性をつける。",
		"ランダムにテレポートする突然変異か、自分の意思でテレポートする突然変異が身につく。",
		"一定時間、冷気、炎、電撃、酸のいずれかに対する免疫を得る。",
	},

	{
		"弱い魔法の矢を放つ。",
		"近くの生命のないモンスターを感知する。",
		"一定時間、命中率とACにボーナスを得る。",
		"一定時間、炎への耐性を得る。装備による耐性に累積する。",
		"モンスター1体を恐怖させ、朦朧させる。抵抗されると無効。",
		"地獄のボルトもしくはビームを放つ。",
		"古代の死霊を召喚する。",
		"邪悪な力を持つボールを放つ。善良なモンスターには大きなダメージを与える。",

		"悪魔1体を魅了する。抵抗されると無効",
		"周辺の地形を感知する。",
		"一定時間、地獄への耐性を得る。",
		"プラズマのボルトもしくはビームを放つ。",
		"炎の球を放つ。",
		"武器に炎の属性をつける。",
		"大きな地獄の球を放つ。",
		"悪魔1体を召喚する。",

		"一定時間、テレパシー能力を得る。",
		"恐怖を取り除き、一定時間、炎と冷気の耐性、炎のオーラを得る。耐性は装備による耐性に累積する。",
		"自分を中心とした炎の球を作り出し、床を溶岩に変える。",
		"プラズマの球を放つ。",
		"一定時間、悪魔に変化する。変化している間は本来の種族の能力を失い、代わりに悪魔としての能力を得る。",
		"視界内の全てのモンスターにダメージを与える。善良なモンスターに特に大きなダメージを与える。",
		"因果混乱の球を放つ。",
		"破滅の手を放つ。食らったモンスターはそのときのHPの半分前後のダメージを受ける。",

		"一定時間、ヒーロー気分になる。",
		"一定時間、時間逆転への耐性を得る。",
		"自分を中心としたカオスの球、混乱の球を発生させ、近くのモンスターを魅了する。",
		"全てのペットを強制的に爆破させる。",
		"上級デーモンを召喚する。召喚するには人間('p','h','t'で表されるモンスター)の死体を捧げなければならない。",
		"超巨大な地獄の球を放つ。",
		"自分がダメージを受けることによって対象に呪いをかけ、ダメージを与え様々な効果を引き起こす。",
		"悪魔の王に変化する。変化している間は本来の種族の能力を失い、代わりに悪魔の王としての能力を得、壁を破壊しながら歩く。",
	},

	{
		"視界内の全てのモンスターを減速させる。抵抗されると無効。",
		"命中率とACのボーナスを得る。",
		"轟音のボルトを放つ。",
		"視界内の全てのモンスターを朦朧させる。抵抗されると無効。",
		"体力を少し回復させる。",
		"光源が照らしている範囲か部屋全体を永久に明るくする。",
		"視界内の全てのモンスターを恐怖させる。抵抗されると無効。",
		"ヒーロー気分になる。",

		"近くの罠/扉/階段を感知する。レベル15で全てのモンスター、20で財宝とアイテムを感知できるようになる。レベル25で周辺の地形を感知し、40でその階全体を永久に照らし、ダンジョン内のすべてのアイテムを感知する。この効果は歌い続けることで順に起こる。",
		"視界内の全てのモンスターに対して精神攻撃を行う。",
		"自分のいるマスと隣りのマスに落ちているアイテムを鑑定する。",
		"隠密行動能力を上昇させる。",
		"視界内の全てのモンスターを混乱させる。抵抗されると無効。",
		"視界内の全てのモンスターに対して轟音攻撃を行う。",
		"周囲の死体や骨を生き返す。",
		"視界内の全てのモンスターを魅了する。抵抗されると無効。",

		"自分のいるマスと隣りのマスの壁を溶かす。",
		"酸、電撃、炎、冷気、毒に対する耐性を得る。装備による耐性に累積する。",
		"加速する。",
		"近くのモンスターをテレポートさせる。抵抗されると無効。",
		"視界内の全てのモンスターにダメージを与える。邪悪なモンスターに特に大きなダメージを与える。",
		"視界内の全てのモンスターを減速させ、眠らせようとする。抵抗されると無効。",
		"轟音のビームを放つ。",
		"現在の階を再構成する。",

		"周囲のダンジョンを揺らし、壁と床をランダムに入れ変える。",
		"視界内の全てのモンスターを麻痺させようとする。抵抗されると無効。",
		"自分のいる床の上に、モンスターが通り抜けたり召喚されたりすることができなくなるルーンを描く。",
		"加速し、ヒーロー気分になり、視界内の全てのモンスターにダメージを与える。",
		"強力な回復の歌で、負傷と朦朧状態も全快する。",
		"すべてのステータスと経験値を回復する。",
		"非常に強力でごく小さい轟音の球を放つ。",
		"ダメージを受けなくなるバリアを張る。"
	},

	{
		"2マス離れたところにいるモンスターを攻撃する。",
		"3方向に対して攻撃する。",
		"武器を手元に戻ってくるように投げる。戻ってこないこともある。",
		"火炎耐性のないモンスターに大ダメージを与える。",
		"近くの思考することができるモンスターを感知する。",
		"相手にダメージを与えないが、朦朧とさせる。",
		"相手に攻撃されたときに反撃する。反撃するたびにMPを消費。",
		"攻撃した後、反対側に抜ける。",

		"毒耐性のないモンスターに大ダメージを与える。",
		"生命のない邪悪なモンスターに大ダメージを与えるが、他のモンスターには全く効果がない。",
		"攻撃した相手を後方へ吹き飛ばす。",
		"武器・防具を1つ識別する。レベル45以上で武器・防具の能力を完全に知ることができる。",
		"岩を壊し、岩石系のモンスターに大ダメージを与える。",
		"攻撃回数が増え、冷気耐性のないモンスターに大ダメージを与える。",
		"モンスターを一撃で倒す攻撃を繰り出す。失敗すると1点しかダメージを与えられない。",
		"会心の一撃で攻撃する。攻撃がかわされやすい。",

		"強力な攻撃を繰り出す。次のターンまでの間、食らうダメージが増える。",
		"電撃耐性のないモンスターに非常に大きいダメージを与える。",
		"素早く相手に近寄り攻撃する。",
		"自分自身も傷を作りつつ、その傷が深いほど大きい威力で全方向の敵を攻撃できる。生きていないモンスターには効果がない。",
		"地震を起こす。",
		"衝撃波のビームを放つ。",
		"視界内の全モンスターに対して轟音の攻撃を行う。さらに、近くにいるモンスターを怒らせる。",
		"強力な3段攻撃を繰り出す。",

		"攻撃した相手の体力を吸いとり、自分の体力を回復させる。生命を持たないモンスターには通じない。",
		"視界内の起きている全モンスターに朦朧、混乱、眠りを与えようとする。",
		"連続して入身でモンスターを攻撃する。攻撃するたびにMPを消費。MPがなくなるか、モンスターを倒せなかったら百人斬りは終了する。",
		"視界内の場所を指定して、その場所と自分の間にいる全モンスターを攻撃し、その場所に移動する。",
		"1ターンで2度攻撃を行う。",
		"強力な攻撃を行い、近くの場所にも効果が及ぶ。",
		"自分もダメージをくらうが、相手に非常に大きなダメージを与える。アンデッドには特に効果がある。",
		"「武士道とは、死ぬことと見つけたり。」"
	},
};
#else
static char *spell_tips[VALID_REALM][32] =
{
	{
		"detects all evil monsters in the neighborhood.",
		"heals cut and HP a little.",
		"gives bonus to hit and AC for few turns.",
		"removes fear.",
		"lights up nearby area and the inside of a room permanently.",
		"detects traps, doors, and stairs in the neighborhood",
		"heals cut and HP more.",
		"satisfies hunger.",

		"removes normal curses from equipped items.",
		"cure poison status.",
		"heals cut, stun and HP greatly.",
		"gives see invisible for a while.",
		"fires a ball with holy power. hurts evil monsters greatly, but don't effect good monsters.",
		"gives aura which protect you from evil monster's physical attack",
		"Much powerful healing magic. and heals cut and stun completely.",
		"sets a glyph on the floor beneath you. Monsters cannot attack you if you are on a glyph, but can try to break glyph", 

		"damages all demons in sight. Also attempts to scare all evil monsters in sight.",
		"removes normal and heavy curse from equipped items.",
		"damages all undead and demons in sight.",
		"attempts to charm all monsters in sight.",
		"damages all evil monsters in sight.",
		"teleports all evil monsters in sight away unless resisted.",
		"damages all evil monsters in sight. And heals poison, fear, stun and cut and HP almost completely.",
		"creates glyphs in all adjacent squares and under you.",

		"removes fear, and gives bonus to hit and 10 more HP for a while.",
		"gives bonus to hit and AC for a long time.",
		"vanishs all undead monsters in the neighborhood from the current dungeon level unless resisted.",
		"restores all stats and experience",
		"The greatest healing magic. heals all HP, cut and stun.",
		"*identifies* an item.",
		"damages all adjacent monsters with holy power. Damages and attempt to slow, stun, confuse, scare and freeze all monsters in sight. And summon an angel, heals HP, gives heroism and speed.",
		"gives ultimate resistance, bonus to AC and speed."
	},

	{
		"detects all monsters in the neighborhood unless invisible.",
		"teleport short distance.",
		"detects traps, doors, and stairs in the neighborhood.",
		"lights up nearby area and the inside of a room permanently.",
		"attempts to confuse a monster.",
		"teleport long distance.",
		"attempts to sleep a monster.",
		"Recharges staffs, wands ot rods.",

		"maps nearby area.",
		"identifies an item.",
		"attempts to slow a monster.",
		"attempts to sleep all monsters in sight.",
		"attempts to teleport a monster away.",
		"hastes you for a while.",
		"detects all monsters, traps, doors, stairs, treasures and items in the neighborhood.",
		"*identifies* an item.",

		"detects all treasures and items in the neighborhood.",
		"attempts to charm a monster.",
		"gives telepathy for a while.",
		"teleport to a town which you choose in a moment. can only be used outdoors.",
		"gives you useful info regarding your current resistances, the powers of your weapon and maximum limits of your stats.",
		"teleport to up or down stairs in a moment.",
		"recalls player from dungeon to town, or from town to the deepest level of dungeon.",
		"sets a glyph under you. The glyph will explode when a monster moves on it.",
		"proves all monsters' HP, speed and their true character.",
		"pulls a distant item close to you.",
		"maps and lights whole dungeon level. knows all objects location. and gives telepathy for a while.",
		"attempts to charm all monsters in sight.",
		"teleport to given location.",
		"turns an item into 1/3 of its value in gold.",
		"teleports all monsters in sight away unless resisted.",
		"generates barrier which completly protect you from almost all damages. takes a few your turns when the barrier breaks or duration time is exceeded.",
	},

	{
		"detects all monsters in the neighborhood unless invisible.",
		"fires a short beam of lightning.",
		"detects traps, doors, and stairs in the neighborhood.",
		"satisfis hunger.",
		"lights up nearby area and the inside of a room permanently.",
		"attempts to charm an animal.",
		"gives resistance to fire, cold and electricity for a while. These resistances can be added to which from equipment for more powerful resistances.",
		"heals all cut and poison status. heals HP a little.",

		"turns one rock square to mud.",
		"fires a bolt or beam of cold",
		"maps nearby area. detects all monsters, traps, doors and stairs.",
		"fires a bolt or beam of fire",
		"fires a beam of light which damages to light-sensitive monsters.",
		"attempts to slow all monsters in sight.",
		"summons an animal.",
		"heals HP grealty. and heals cut, stun and poison completely.",

		"creates a stair which goes down or up.",
		"gives bonus to AC for a while.",
		"gives resistance to fire, cold, electricity, acid and poison for a while. These resistances can be added to which from equipment for more powerful resistances.",

		"creates trees in all adjacent squares.",
		"attempts to charm all animals in sight.",
		"*identifies* an item.",
		"creates granite walls in all adjacent squares.",
		"makes an equipment acid-proof.",

		"shakes dungeon structure, and results in random swaping of floors and walls.",
		"attacks all adjacent monsters.",
		"fires a huge ball of cold.",
		"fires a huge electric ball.",
		"fires a huge ball of water.",
		"generates ball of light centerd on you. maps and lights whole dungeon level. knows all objects location.",
		"makes current weapon fire or frost branded.",
		"damages all monsters in sight. makes quake. generates disintegration ball centerd on you.",
	},

	{
		"fires a weak bolt of magic.",
		"destroys all traps in adjacent squares.",
		"lights up nearby area and the inside of a room permanently.",
		"attempts to confuse the next monster that you hit.",
		"fires a ball of magic.",
		"fires a bolt or beam of fire",
		"fires a tiny ball of disintegration.",
		"teleport long distance.",

		"fires someting with random effects",
		"fires a bolt or ball of chaos.",
		"generates a ball of sound centered on you.",
		"fires a beam of pure mana.",
		"fires a ball of fire.",
		"attempts to teleport a monster away.",
		"destroy everything in nearby area.",
		"fires a huge ball of chaos.",

		"attempts to polymorph a monster.",
		"fires lightning beams in all directions.",
		"Recharges staffs, wands ot rods.",
		"fires a huge ball of disintegration.",
		"recreates current dungeon level.",
		"fires a magic rocket",
		"makes current weapon a Chaotic weapon.",
		"summons a demon.",

		"fires a beam of gravity.",
		"makes meteor balls fall down to nearby random locations.",
		"generate a huge ball of fire centerd on you.",
		"generate random kind of balls or beams.",
		"polymorphs yourself.",
		"fires an extremely powerful huge ball of pure mana.",
		"fires an extremely powerful ball of chaos.",
		"fires rochets, mana balls and nuclear waste balls in all directions each unless you are not adjacent to any walls. Otherwise *destroys* huge area.",
	},

	{
		"detects all nonliving monsters in the neighborhood.",
		"fires a tiny ball of evil power which hurts good monsters grealty.",
		"detects all evil monsters in the neighborhood.",
		"fires a ball of poison.",
		"attempts to sleep a monster.",
		"gives resistance to poison. This resistance can be added to which from equipment for more powerful resistance.",
		"attempts to scare and stun a monster.",
		"attempts to charm an undead monster.",

		"fires a ball which damages living monsters.",
		"fires a bolt or beam of nether.",
		"generate a ball of poison centerd on you.",
		"attempts to vanish a monster.",
		"makes current weapon poison branded.",
		"absorbs some HP from a monster and gives them to you. You will also gain nutritional sustenance from this.",
		"resurrects nearby corpse and skeltons. and makes these your pets.",
		"removes all monsters with given symbol from the current dungeon level unless resisted.",

		"gives bonus to hit and HP, immunity to fear for a while. but decreases AC.",
		"causes random effects.",
		"fires a bolt or beam of darkness.",
		"gives another bonus to hit and HP, immunity to fear for a while. hastes you. but decreases AC.",
		"makes current weapon Vamparic.",
		"fires 3 bolts. Each of the bolts absorbs some HP from a monster and gives them to you.",
		"damages all living monsters in sight.",
		"fires a huge ball of darkness.",

		"fires a beam of death.",
		"summons an undead monster.",
		"identifies an item. or *identifies* an item at higher level.",
		"mimic a vampire for a while. loses abilities of original race and gets abilitis as a vampire.",
		"restore lost experience.",
		"removes all monsters in the neighborhood from the current dungeon level  unless resisted.",
		"fires a powerful ball of evil power. hurts good monsters greatly.",
		"becomes wraith form which gives ability to pass walls and makes all damages half.",
	},

	{
		"teleport short distance.",
		"summons a spider.",
		"causes random effects.",
		"reset the 'deepest' level for recall spell.",
		"teleport long distance.",
		"gives telepathy for a while.",
		"attempts to teleport a monster away.",
		"attempts to charm an animal.",

		"pulls a distant item close to you.",
		"summons monsters which explode by itself.",
		"summons a ghost.",
		"hastes a monster.",
		"teleport to up or down stairs in a moment.",
		"teleport to given location.",
		"recalls player from dungeon to town, or from town to the deepest level of dungeon.",
		"teleports all monsters in sight away unless resisted.",

		"swap positions of you and a monster.",
		"summons an undead monster.",
		"summons a hydra.",
		"summons some monsters.",
		"summons a group of hounds.",
		"makes current weapon a Trump weapon.",
		"gives mutation which makes you teleport randomly or makes you able to teleport at will.",
		"summons a cyber demon.",

		"detects all monsters, traps, doors, stairs, treasures and items in the neighborhood.",
		"*identifies* an item.",
		"heal a monster.",
		"summons a dragon.",
		"makes meteor balls fall down to nearby random locations.",
		"summons a demon.",
		"summons a greater undead.",
		"summons an ancient dragon.",
	},

	{
		"fires a bolt or beam of lightning.",
		"locks a door.",
		"detects all invisible monsters in the neighborhood.",
		"detects all monsters in the neighborhood unless invisible.",
		"teleport short distance.",
		"lights up nearby area and the inside of a room permanently.",
		"fires a beam which destroy traps and doors.",
		"heals cut and HP a little.",

		"detects traps, doors, and stairs in the neighborhood.",
		"adds more turns of light to a lantern or torch.",
		"detects all treasures in the neighborhood.",
		"detects all magical items in the neighborhood.",
		"detects all items in the neighborhood.",
		"cures poison status.",
		"gives resistance to cold. This resistance can be added to which from equipment for more powerful resistance.",
		"gives resistance to fire. This resistance can be added to which from equipment for more powerful resistance.",

		"gives resistance to electricity. This resistance can be added to which from equipment for more powerful resistance.",
		"gives resistance to acid. This resistance can be added to which from equipment for more powerful resistance.",
		"heals cut and HP more.",
		"teleport long distance.",
		"identifies an item.",
		"turns one rock square to mud.",
		"fires a beam of light which damages to light-sensitive monsters.",
		"satisfis hunger.",
		"gives see invisible for a while.",

		"summons an elemental.",
		"teleport to up or down stairs in a moment.",
		"attempts to teleport a monster away.",
		"fires a ball of some elements.",
		"detects all monsters, traps, doors, stairs, treasures and items in the neighborhood.",
		"recalls player from dungeon to town, or from town to the deepest level of dungeon.",
		"maps and lights whole dungeon level. knows all objects location. and gives telepathy for a while.",
	},

	{
		"gives infravision for a while.",
		"gives regeneration ability for a while.",
		"satisfis hunger.",
		"gives resistance to cold. This resistance can be added to which from equipment for more powerful resistance.",
		"gives resistance to fire. This resistance can be added to which from equipment for more powerful resistance.",
		"removes fear, and gives bonus to hit and 10 more HP for a while.",
		"gives resistance to electricity. This resistance can be added to which from equipment for more powerful resistance.",
		"gives resistance to acid. This resistance can be added to which from equipment for more powerful resistance.",

		"gives see invisible for a while.",
		"removes normal curses from equipped items.",
		"gives resistance to poison. This resistance can be added to which from equipment for more powerful resistance.",
		"gives bonus to hit and HP, immunity to fear for a while. but decreases AC.",
		"gives you useful info regarding your current resistances, the powers of your weapon and maximum limits of your stats.",
		"gives aura which protect you from evil monster's physical attack",
		"heals poison, stun, cut and hullucination completely.",
		"makes current weapon some elemental branded.",

		"gives telepathy for a while.",
		"gives bonus to AC for a while.",
		"gives resistance to fire, cold, electricity, acid and poison for a while. These resistances can be added to which from equipment for more powerful resistances.",
		"hastes you for a while.",
		"gives ability to pass walls for a while.",
		"makes a shild a shild of reflection.",
		"creates a golem.",
		"gives resistance to magic, bonus to AC, resistance to confusion, blindness, reflection, free action and levitation for a while.",

		"removes all magics completely from any weapon or armor.",
		"removes normal and heavy curse from equipped items.",
		"*identifies* an item.",
		"attempts to increase +to-hit, +to-dam of a weapon.",
		"attempts to increase +AC of an armor.",
		"makes current weapon a random ego weapon.",
		"gives mutation which makes you teleport randomly or makes you able to teleport at will.",
		"gives an immunity to fire, cold, electricity or acid for a while.",
	},

	{
		"fires a weak bolt of magic.",
		"detects all nonliving monsters in the neighborhood.",
		"gives bonus to hit and AC for few turns.",
		"gives resistance to fire, cold and electricity for a while. These resistances can be added to which from equipment for more powerful resistances.",
		"attempts to scare and stun a monster.",
		"fires a bolt or beam of nether.",
		"summons a manes.",
		"fires a ball of evil power. hurts good monsters greatly.",

		"attempts to charm a demon.",
		"maps nearby area.",
		"gives resistance to nether for a while.",
		"fires a bolt or beam of plasma.",
		"fires a ball of fire.",
		"makes current weapon fire branded.",
		"fires a huge ball of nether.",
		"summons a demon.",

		"gives telepathy for a while.",
		"removes fear. gives resistance to fire and cold, and aura of fire. These resistances can be added to which from equipment for more powerful resistances.",
		"generates a ball of fire centerd on you which transforms floors to magma.",
		"fires a ball of plasma.",
		"mimic a demon for a while. loses abilities of original race and gets abilitis as a demon.",
		"damages all monsters in sight. hurts good monsters greatly.",
		"fires a ball of nexus.",
		"attempts to make a monster's HP almost half.",

		"removes fear, and gives bonus to hit and 10 more HP for a while.",
		"gives resistance to time for a while.",
		"generate balls of chaos, confusion and charm centerd on you.",
		"makes all pets explode.",
		"summons greater demon. it need to sacrifice a corpse of human ('p','h' or 't').",
		"generate a huge ball of nether.",
		"puts blood curse which damages and causes various effects on a monster. You also take damage.",
		"mimic a demon lord for a while. loses abilities of original race and gets great abilitis as a demon lord. Even hard walls can't stop your walking",
	},

	{
		"attempts to slow all monsters in sight.",
		"gives bonus to hit and AC for few turns.",
		"fires a bolt of sound.",
		"attempts to stun all monsters in sight.",
		"heals HP a little.",
		"lights up nearby area and the inside of a room permanently.",
		"attempts to scare all monsters in sight.",
		"removes fear, and gives bonus to hit and 10 more HP for a while.",
		
		"detects traps, doors and stairs in the neighborhood. and detects all monsters at level 15, treasures and items at level 20. maps nearby area at level 25. lights and know the whole level at level 40. These effects occures by turns while this song continues.",
		"damages all monsters in sight with PSI damages.",
		"identifies all items which are in the adjacent squares.",
		"gives improved stalth for a while.",
		"attempts to confuse all monsters in sight.",
		"damages all monsters in sight with booming sound.",
		"resurrects nearby corpse and skeltons. and makes these your pets.",
		"attempts to charm all monsters in sight.",

		"turns all rocks in the adjacent squares to mud.",
		"gives resistance to fire, cold, electricity, acid and poison for a while. These resistances can be added to which from equipment for more powerful resistances.",
		"hastes you.",
		"teleports all monsters in sight away unless resisted.",
		"damages all monsters in sight. hurts evil monsters greatly.",
		"attempts to slow and sleep all monsters in sight.",
		"fires a beam of sound.",
		"recreates current dungeon level.",

		"shakes dungeon structure, and results in random swaping of floors and walls.",
		"attempts to freeze all monsters in sight.",
		"sets a glyph on the floor beneath you. Monsters cannot attack you if you are on a glyph, but can try to break glyph", 
		"hastes you. gives hiroism. damages all monsters in sight.",
		"Powerful healing song. also heals cut and stun completly.",
		"restores all stats and experience",
		"fires an extremely powerful tiny ball of sound.",
		"generates barrier which completly protect you from almost all damages. takes a few your turns when the barrier breaks.",
	},

	{
		"attacks a two squares distant monster.",
		"attacks in 3 directions in one time.",
		"throws current weapon. and it'll return to your hand unless failed.",
		"attacks a monster with more damage unless it has resistance to fire.",
		"detects all monsters except mindless in the neighborhood.",
		"attempts to stun a monster in the adjacent.",
		"prepares to counterattack. when attack by a monster, strikes back using SP each time.",
		"attacks monster with your weapons normaly, then move through counter side of the monster.",

		"attacks a monster with more damage unless it has resistance to poison.",
		"attacks an unliving monster with greate damage. No effect to other  monsters.",
		"attacks an adjacent monster, and blow it away.",
		"identifies a weapon or armor. or *identifies* these at level 45.",

		"breaks rock. or greatly damage a monster made by rocks.",
		"attacks a monster with increased number of attacks and more damage unless it has resistance to cold.",
		"attempts to kill a monster instantly. If failed cause only 1HP of damage.",
		"attempts to attack with critical hit. But this attack is easy to evade for a monster.",

		"attacks with all of your power. but all damages you take will be doubled for one turn.",
		"attacks a monster with more damage unless it has resistance to electricity.",
		"steps close to a monster and attacks at a time.",
		"attacks all adjacent monsters with power corresponding to your cut status. then increases your cut status. No effect to unliving monsters.",
		"shakes dungeon structure, and results in random swaping of floors and walls.",
		"fires a beam of shock wave.",
		"damages all monsters in sight with sound. aggrevate nearby monsters.",
		"attacks with powerful 3 strikes.",

		"attacks with vamparic strikes which absorbs HP from a monster and gives them to you. No effect to unliving monsters.",
		"attempts to stun, confuse and sleep all waking monster.",
		"performs a series of rush attacks. The series continues while killing each monster in a time and SP remains.",
		"runs toward given location while attacking all monsters on the path.",
		"double attacks at a time.",
		"performs a powerful attack which even effect nearby monsters.",
		"attacks a monster with extremely powerful damage. But you also takes some damages. hurts a undead monster greatly.",
		"'Busido is found in death'"
	},
};
#endif
