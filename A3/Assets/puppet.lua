-- Simple Scene:
-- An extremely simple scene that will render right out of the box
-- with the provided skeleton code.  It doesn't rely on hierarchical
-- transformations.

-- Create the top level root node named 'root'.
rootNode = gr.node('root')

-- Reference image for colors ( and modelling )
-- https://upload.wikimedia.org/wikipedia/en/thumb/3/3b/SpongeBob_SquarePants_character.svg/1200px-SpongeBob_SquarePants_character.svg.png

red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
blue = gr.material({0.28, 0.90, 1.2}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)
brown = gr.material({0.8,0.5, 0.25}, {0.1, 0.1, 0.1}, 10)
darkbrown = gr.material({0.21, 0.14, 0.02}, {0.1, 0.1, 0.1}, 10)
slightDarkbrown = gr.material({0.70, 0.47, 0.23}, {0.1, 0.1, 0.1}, 10)
black = gr.material({0.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.2, 1.2, 1.2}, {0.5, 0.5, 0.5}, 10)
yellow = gr.material({1.5, 1.4, 0.45}, {0.1, 0.1, 0.1}, 20)

---------------------------------------torso--------------------------------------
torso = gr.mesh('cube', 'torso')
torso:scale(2.0, 0.7, 0.5)
-- torso:rotate('y', 45.0)
torso:translate(0.0, -0.5, -5.0)
torso:set_material(white)

rootNode:add_child(torso)

---------------------------------NeckNode-----------------------------
neckNode = gr.node('neckNode')
torso:add_child(neckNode)
neckNode:scale(1, 1.0/0.9, 1.0/0.5)
neckNode:translate(0.0, 0.5, 0.0)

tie1 = gr.mesh('sphere', 'tie1')
tie1:scale(0.1, 0.1, 0.02)
tie1:translate(0.0, -0.62, 0.25)
tie1:set_material(red)

neckNode:add_child(tie1)

tie2 = gr.mesh('cube', 'tie2')
tie2:scale(1, 2, 1)
tie2:translate(0.0, -1.7, 0.0)
tie2:set_material(red)

tie1:add_child(tie2)


---------------------------------NeckJoint-----------------------------
-- he's basically all neck so it's pretty stiffy here
neckJoint = gr.joint('neckJoint', {-5, 0, 5}, {-5, 0, 5});
neckNode:add_child(neckJoint)

---------------------------------Head-----------------------------
head = gr.mesh('cube', 'head')
head:scale(1.05, 2.5, 0.6)
head:translate(0.0, 0.7, 0.0)
head:set_material(yellow)

neckJoint:add_child(head)

--------------------------------Face---------------------------
leftEye = gr.mesh('sphere', 'outerLeftEye')
leftEye:scale(1/1.0 * 0.15,1/0.9 * 0.15,1/0.5 * 0.012)
leftEye:translate(-0.13, 0.15, 0.52)
leftEye:set_material(white)

head:add_child(leftEye)

rightEye = gr.mesh('sphere', 'outerRightEye')
rightEye:scale(1/1.0 * 0.15,1/0.9 * 0.15,1/0.5 * 0.01)
rightEye:translate(0.13, 0.15, 0.5)
rightEye:set_material(white)

head:add_child(rightEye)

-- Inner eye
innerEyeFactor = 0.3
innerLeftEye = gr.mesh('sphere', 'innerLeftEye')
innerLeftEye:scale(innerEyeFactor,innerEyeFactor,2 * innerEyeFactor)
innerLeftEye:translate(0.2, -0.2, 0.8)
innerLeftEye:set_material(blue)

leftEye:add_child(innerLeftEye)

innerRightEye = gr.mesh('sphere', 'innerRightEye')
innerRightEye:scale(innerEyeFactor,innerEyeFactor,2 * innerEyeFactor)
innerRightEye:translate(-0.2, -0.2, 0.8)
innerRightEye:set_material(blue)

rightEye:add_child(innerRightEye)

-- Corona
coronaFactor = 0.55
leftCorona = gr.mesh('sphere', 'leftCorona')
leftCorona:scale(coronaFactor,coronaFactor,2 * coronaFactor)
leftCorona:translate(0.1, 0, 0.8)
leftCorona:set_material(black)

innerLeftEye:add_child(leftCorona);

rightCorona = gr.mesh('sphere', 'rightCorona')
rightCorona:scale(coronaFactor,coronaFactor,2 * coronaFactor)
rightCorona:translate(0.1, 0, 0.8)
rightCorona:set_material(black)

innerRightEye:add_child(rightCorona);

nose = gr.mesh('sphere', 'nose')
nose:scale(0.05, 0.05, 0.2)
nose:translate(0, -0.05, 0.5)
nose:rotate('y', 5.0)
nose:rotate('z', -5.0)
nose:set_material(yellow)

head:add_child(nose)

---------------------------------WaistNode---------------------------
waistNode = gr.node('waistNode')
torso:add_child(waistNode)
waistNode:scale(1.0/0.8, 1.0/0.9, 1.0/0.5) -- now scale(1,1,1)
waistNode:translate(0.0, -0.3, 0.0)

---------------------------------WaistJoint-----------------------------
waistJoint = gr.joint('waistJoint', {-10,0, 10}, {0, 0, 0});
waistNode:add_child(waistJoint)

---------------------------------Waist-----------------------------
waist = gr.mesh('cube', 'waist')
waist:scale(0.78, 0.7, 0.4)
waist:translate(0.0, -0.3, 0)
waist:set_material(slightDarkbrown)

waistJoint:add_child(waist)

---------------------------------LeftThighNode---------------------------
leftThighNode = gr.node('leftThighNode')
waist:add_child(leftThighNode)
leftThighNode:scale(1.0/0.8, 1.0/0.5, 1.0/0.5) 
leftThighNode:translate(0.0, -0.3, 0.0)
---------------------------------LeftThighJoint-----------------------------
leftThighJoint = gr.joint('leftThighJoint', {-20, 0, 20}, {0, 0, 0});
leftThighNode:add_child(leftThighJoint)

---------------------------------LeftThigh-----------------------------
leftThigh = gr.mesh('cube', 'leftThigh')
leftThigh:scale(0.2, 0.3, 0.2)
leftThigh:translate(-0.2, -0.15, 0.0)
leftThigh:set_material(brown)

leftThighJoint:add_child(leftThigh)

---------------------------------LeftShankNode---------------------------
leftShankNode = gr.node('leftShankNode')
leftThigh:add_child(leftShankNode)
leftShankNode:scale(1.0/0.3, 1.0/0.4, 1.0/0.2) -- now scale(1,1,1)
leftShankNode:translate(0.0, -0.1, 0.0)

---------------------------------LeftShankJoint-----------------------------
leftShankJoint = gr.joint('leftShankJoint', {-12, 0, 12}, {5, 0, 5});
leftShankNode:add_child(leftShankJoint)


---------------------------------LeftShank-----------------------------
leftShank = gr.mesh('cube', 'leftShank')
leftShank:scale(0.13, 2, 0.13)
leftShank:translate(0.0, -0.7, 0.0)
leftShank:set_material(yellow)

leftShankJoint:add_child(leftShank)

---------------------------------LeftFootNode---------------------------
leftFootNode = gr.node('leftFootNode')
leftShank:add_child(leftFootNode)
leftFootNode:scale(1.0/0.3, 1.0/0.5, 1.0/0.2)
leftFootNode:translate(0.0, 0.0, 0.0)

---------------------------------LeftFootJoint-----------------------------
leftFootJoint = gr.joint('leftFootJoint', {-10, 0, 10}, {0, 0, 0});
leftFootNode:add_child(leftFootJoint)

-------------------------------- Left Sock ----------------------
leftSock = gr.mesh('cube', 'leftSock')
leftSock:scale(1.1, 0.5, 1.1)
leftSock:translate(0.0, -0.2, 0.0)
leftSock:set_material(white)

leftShank:add_child(leftSock)

---------------------------------LeftFoot-----------------------------
leftFoot = gr.mesh('sphere', 'leftFoot')
leftFoot:scale(0.3, 0.05, 0.3)
leftFoot:translate(0.0, -0.23, 0.0)
leftFoot:set_material(black)

leftFootJoint:add_child(leftFoot)


---------------------------------RightThighNode---------------------------
rightThighNode = gr.node('rightThighNode')
waist:add_child(rightThighNode)
rightThighNode:scale(1.0/0.8, 1.0/0.5, 1.0/0.5) 
rightThighNode:translate(0.0, -0.3, 0.0)
---------------------------------RightThighJoint-----------------------------
rightThighJoint = gr.joint('rightThighJoint', {-20, 0, 20}, {0, 0, 0});
rightThighNode:add_child(rightThighJoint)

---------------------------------RightThigh-----------------------------
rightThigh = gr.mesh('cube', 'rightThigh')
rightThigh:scale(0.2, 0.3, 0.2)
rightThigh:translate(0.2, -0.15, 0.0)
rightThigh:set_material(brown)

rightThighJoint:add_child(rightThigh)

---------------------------------RightShankNode---------------------------
rightShankNode = gr.node('rightShankNode')
rightThigh:add_child(rightShankNode)
rightShankNode:scale(1.0/0.3, 1.0/0.4, 1.0/0.2) -- now scale(1,1,1)
rightShankNode:translate(0.0, -0.1, 0.0)

---------------------------------RightShankJoint-----------------------------
rightShankJoint = gr.joint('rightShankJoint', {-12, 0, 12}, {5, 0, 5});
rightShankNode:add_child(rightShankJoint)


---------------------------------RightShank-----------------------------
rightShank = gr.mesh('cube', 'rightShank')
rightShank:scale(0.13, 2, 0.13)
rightShank:translate(0.0, -0.7, 0.0)
rightShank:set_material(yellow)

rightShankJoint:add_child(rightShank)

---------------------------------RightFootNode---------------------------
rightFootNode = gr.node('rightFootNode')
rightShank:add_child(rightFootNode)
rightFootNode:scale(1.0/0.3, 1.0/0.5, 1.0/0.2)
rightFootNode:translate(0.0, 0.0, 0.0)

---------------------------------RightFootJoint-----------------------------
rightFootJoint = gr.joint('rightFootJoint', {-10, 0, 10}, {0, 0, 0});
rightFootNode:add_child(rightFootJoint)

-------------------------------- Right Sock ----------------------
rightSock = gr.mesh('cube', 'rightSock')
rightSock:scale(1.1, 0.5, 1.1)
rightSock:translate(0.0, -0.2, 0.0)
rightSock:set_material(white)

rightShank:add_child(rightSock)

---------------------------------RightFoot-----------------------------
rightFoot = gr.mesh('sphere', 'rightFoot')
rightFoot:scale(0.3, 0.05, 0.3)
rightFoot:translate(0.0, -0.23, 0.0)
rightFoot:set_material(black)

rightFootJoint:add_child(rightFoot)

---------------------------------LeftUpperArmNode---------------------------
leftUpperArmNode = gr.node('leftUpperArmNode')
torso:add_child(leftUpperArmNode)
leftUpperArmNode:scale(1.0/0.8, 1.0/0.9, 1.0/0.5) -- now scale(1,1,1)
leftUpperArmNode:translate(-0.4, 0.0, 0.0)
---------------------------------LeftUpperArmJoint-----------------------------
leftUpperArmJoint = gr.joint('leftUpperArmJoint', {-20, 0, 20}, {-10, 0, 10});
leftUpperArmNode:add_child(leftUpperArmJoint)

---------------------------------LeftUpperArm-----------------------------
leftUpperArm = gr.mesh('sphere', 'leftUpperArm')
leftUpperArm:scale(0.1, 0.2, 0.1)
leftUpperArm:translate(-0.1, -0.3, 0.0)
leftUpperArm:set_material(white)

leftUpperArmJoint:add_child(leftUpperArm)

---------------------------------LeftLowerArmNode---------------------------
leftLowerArmNode = gr.node('leftLowerArmNode')
leftUpperArm:add_child(leftLowerArmNode)
leftLowerArmNode:scale(1.0/0.3, 1.0/0.4, 1.0/0.2) -- now scale(1,1,1)
leftLowerArmNode:translate(0.0, -0.5, 0.0)

---------------------------------LeftLowerArmJoint-----------------------------
leftLowerArmJoint = gr.joint('leftLowerArmJoint', {-60, 0, 10}, {0, 0, 0});
leftLowerArmNode:add_child(leftLowerArmJoint)


---------------------------------LeftLowerArm-----------------------------
leftLowerArm = gr.mesh('cube', 'leftLowerArm')
leftLowerArm:scale(0.2, 2, 0.2)
leftLowerArm:translate(0.0, -0.855, 0.0)
leftLowerArm:set_material(yellow)

leftLowerArmJoint:add_child(leftLowerArm)

---------------------------------LeftHandNode---------------------------
leftHandNode = gr.node('leftHandNode')
leftLowerArm:add_child(leftHandNode)
leftHandNode:scale(1.0/0.2, 1.0/2, 1.0/0.2)
leftHandNode:translate(0.0, -0.3, 0.0)

---------------------------------LeftHandJoint-----------------------------
leftHandJoint = gr.joint('leftHandJoint', {-15, 0, 15}, {0, 0, 0});
leftHandNode:add_child(leftHandJoint)


---------------------------------LeftHand-----------------------------
leftHand = gr.mesh('sphere', 'leftHand')
leftHand:scale(0.25, 0.25, 0.25)
leftHand:translate(0.0, -0.2, 0.0)
leftHand:set_material(yellow)

leftHandJoint:add_child(leftHand)

---------------------------------RightUpperArmNode---------------------------
rightUpperArmNode = gr.node('rightUpperArmNode')
torso:add_child(rightUpperArmNode)
rightUpperArmNode:scale(1.0/0.8, 1.0/0.9, 1.0/0.5) -- now scale(1,1,1)
rightUpperArmNode:translate(0.4, 0.0, 0.0)

---------------------------------RightUpperArmJoint-----------------------------
rightUpperArmJoint = gr.joint('rightUpperArmJoint', {-60, 0, 60}, {-30, 0, 30});
rightUpperArmNode:add_child(rightUpperArmJoint)

---------------------------------RightUpperArm-----------------------------
rightUpperArm = gr.mesh('sphere', 'rightUpperArm')
rightUpperArm:scale(0.1, 0.2, 0.1)
rightUpperArm:translate(0.1, -0.3, 0.0)
rightUpperArm:set_material(white)
rightUpperArmJoint:add_child(rightUpperArm)

---------------------------------RightLowerArmNode---------------------------
rightLowerArmNode = gr.node('rightLowerArmNode')
rightUpperArm:add_child(rightLowerArmNode)
rightLowerArmNode:scale(1.0/0.3, 1.0/0.4, 1.0/0.2) -- now scale(1,1,1)
rightLowerArmNode:translate(0.0, -0.5, 0.0)


---------------------------------RightLowerArmJoint-----------------------------
rightLowerArmJoint = gr.joint('rightLowerArmJoint', {-60, 0, 10}, {0, 0, 0});
rightLowerArmNode:add_child(rightLowerArmJoint)


---------------------------------RightLowerArm-----------------------------
rightLowerArm = gr.mesh('cube', 'rightLowerArm')
rightLowerArm:scale(0.2, 2, 0.2)
rightLowerArm:translate(0.0, -0.855, 0.0)
rightLowerArm:set_material(yellow)

rightLowerArmJoint:add_child(rightLowerArm)

---------------------------------RightHandNode---------------------------
rightHandNode = gr.node('rightHandNode')
rightLowerArm:add_child(rightHandNode)
rightHandNode:scale(1.0/0.2, 1.0/2, 1.0/0.2) -- now scale(1,1,1)
rightHandNode:translate(0.0, -0.3, 0.0)

---------------------------------RightHandJoint-----------------------------
rightHandJoint = gr.joint('rightHandJoint', {-15, 0, 15}, {0, 0, 0});
rightHandNode:add_child(rightHandJoint)


---------------------------------RightHand-----------------------------
rightHand = gr.mesh('sphere', 'rightHand')
rightHand:scale(0.25, 0.25, 0.25)
rightHand:translate(0.0, -0.2, 0.0)
rightHand:set_material(yellow)

rightHandJoint:add_child(rightHand)

return rootNode;