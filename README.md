# Combat Prototype

Developed with Unreal Engine 5.4

- [Playable Windows Version](https://maisumpedro.itch.io/combat-prototype)

## Objective

Develop a prototype for a Game Jam in 2 weeks. The main focus was to implement combat mechanics inspired by For Honor, such as attacking, blocking, and parrying with directional commands.

This Game Jam had two rules: don't use code from others and use the set of animations Sword Animset Pro.

## Combat Mechanics

### Attack
<div>
	<img src="./docs/cp_attack.gif" />
</div>

### Block
<div>
    <img src="./docs/cp_blocks.gif" />
</div>

### Parry
<div>
    <img src="./docs/cp_parry.gif" />
</div>

## Details

In this prototype, I developed some mechanics focusing on the entire gameplay. To play the prototype, this includes:
- Character
    - Stats: Health
    - Actions: Walk, join/exit combat mode, and directional attack, block, and parry.
    - Animations: Configure montages and animation blueprints, both with root motion animations.
- Enemy
    - Same actions and stats as the character
    - AI: Follow and attack
- Interface
    - A directional pose with attack alerts on enemies
    - Some tips to learn the commands
    - A start and pause menu
- Others
    - A system using a data asset to configure a list of attacks and each attack's info, including montages and dependencies, like which attack needs to be made for this attack to happen.

## Credits

Code by Pedro Oliveira (me), and the animations are from the package that I mentioned. The sparks and blood VFX are from the "Realistic Starter VFX Pack Vol. 2". All links are below.

Regarding the code, I am only sharing my C++ code because the rest is under the Epic Games License and I can't share it on a public repository.

## Assets from Fab Marketplace
- Animations: [Sword Animset Pro](https://www.fab.com/listings/12783d2f-2bcf-4f3e-9a4a-d554cc3c579a)
- VFX: [Realistic Starter VFX Pack Vol. 2](https://www.fab.com/listings/ac2818b3-7d35-4cf5-a1af-cbf8ff5c61c1)

## Unreal Engine Links

- Link to download: https://www.unrealengine.com/pt-BR/download
- Terms: https://www.unrealengine.com/pt-BR/eula-reference/unreal-pt-br