python to_kmodel.py --target k230 --model C:\Users\13175\Desktop\project\maxarm\yolo11\ultralytics-8.3.167\runs\train\exp2\weights\best.onnx --dataset E:\datasets_images\arm_v2\res\images\val --input_width 320 --input_height 320 --ptq_option 0

要想不每次加载预训练模型，就必须amp=False!!!