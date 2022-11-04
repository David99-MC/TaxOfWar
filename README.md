# TaxOfWar
 Story/quest-driven RPG open-world 
 
 Here's the file for a demo: https://drive.google.com/file/d/1-t4KFp8TH9yUsyTWNMvAoh94SvvlNd1R/view?usp=sharing
 
 Steps to download the game:
 + Download and unzip the TaxOfWar file
 + Open Window file
 + Double click on TaxOfWar.exe to open the game
 + Enjoy!

This project's purpose is to get me used to Unreal Engine. I've learnt so much by putting this project together, I will give a summary of what I learnt below:
1. Utilze inheritance to:
  + share similar functionalities between the Player and the enemies.
  + share similar functionalities between the Items: Weapon, Pickup, and Hazardous elements (i.e. explosives).
  
2. The animation blueprint: I spent a good amount of time playing around with animation blueprint in this project
  Setting up animations:
    Using a blendspace for idle/movement since I want the animation’s transition to be blended between each other.
    + Use animation blueprint to:
    Hold the idle/movement blendspace
    Manage AnimNotify - events that are called in the animation..
    Update the values used to determine the animation in idle/movement blendspace.
    => Can be put in the CharacterMesh to enable animation.
    + Use AnimMontage to manage input actions:
    AnimMontage allows us to play a specific section in the AnimMontage when received an input:
      + Attack
      + Rolling
      + Get Hit
      + Die
    => All animations in one place, making it easier to make any upcoming changes.
    => Add any AnimNotify directly into the AnimMontage, which can be called and executed in the animation blueprint.

3. Setting up chain attack mechanics for the Player.
    
4. Setting HUD: Displaying Health bars for the enemies and the OverlayHUD for the Player's Health and Stamina.

5. A Stamina system that drains when the Player sprint, roll, and attack.

6. Equipping Weapons such as TwoHandSword and Bow/Arrow and switching animations based on the weapon.

7. Enemy Combat mechanics:
   + Move towards the Player when they are close enough that is detected through a Sphere Collision and Attack when within attack range.
   + Delay the Enemies attack using a timer delegate.
   + Enemies can flexibly change between close range and ranged attacks, each type will play the correct animations.
  
8. UParticleSystem vs UParticleSystemComponent:
 + ParticleSystem: 
   + Used for playing a short particle which is triggered through UGameplayStatics::SpawnEmitterAtLocation();
   + Usually triggered when getting hit, quick interactions, etc.
   + Doesn't need to be created through CreateDefaultSubobject<>() which then attached to the RootComponent.
 + ParticleSystemComponent:
   + Used when need to play over longer period of time.
   + Has to be created through CreateDefaultSubobject<>() which then attached to the RootComponent (if need to play a consistent particle effects on an Actor).
   + Usually used for playing an effects within the map (i.e. falling leaves, snow drops).
   + Can be played as a consistent effect on an Actor (i.e. flame sword, flame armor)
