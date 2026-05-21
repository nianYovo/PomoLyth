#include "ai/MockAiClient.h"

QString MockAiClient::chat(const QString& prompt) {
    if (prompt.contains("review", Qt::CaseInsensitive)) {
        return "本轮复盘：你完成了一次明确的专注推进。下一轮建议继续缩小目标，并优先处理卡点。";
    }

    if (prompt.contains("daily", Qt::CaseInsensitive)) {
        return "今日专注报告：今天已经积累了有效专注记录。建议明天优先安排最需要脑力的任务，并提前关闭常见分心窗口。";
    }

    return "本轮计划：1. 明确最小可完成目标；2. 先完成核心步骤；3. 记录一个卡点。避免事项：不要频繁切换资料，不要临时扩展范围。";
}
