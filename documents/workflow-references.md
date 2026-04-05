# Workflow References

この文書は、workflow、review、agent system、report policy を設計するときに参照した外部資料の索引です。
外部根拠で repo-wide な手順を更新した場合は、この文書へ出典を追記します。

## Agent Runtime And Customization

- [Custom instructions with AGENTS.md - Codex | OpenAI Developers](https://developers.openai.com/codex/guides/agents-md)
  - root `AGENTS.md` を入口にする運用の根拠です。
- [Subagents - Codex | OpenAI Developers](https://developers.openai.com/codex/subagents)
  - Codex subagent の置き方と使い分けの根拠です。
- [How Claude remembers your project - Claude Code Docs](https://code.claude.com/docs/en/memory)
  - `CLAUDE.md` を薄い adapter にする判断の参考です。
- [Copilot customization cheat sheet - GitHub Docs](https://docs.github.com/en/copilot/reference/customization-cheat-sheet)
  - GitHub Copilot 用 adapter と custom instructions の整理に使った資料です。
- [Your first custom instructions - GitHub Docs](https://docs.github.com/en/copilot/tutorials/customization-library/custom-instructions/your-first-custom-instructions)
  - Copilot 側の最小入口設計の参考です。

## System Development, Security, Release, And Operations

- [NIST SP 800-218, Secure Software Development Framework (SSDF)](https://csrc.nist.gov/pubs/sp/800/218/final)
  - secure development workflow、review gate、supply-chain 観点の根拠です。
- [Microsoft Security Development Lifecycle](https://www.microsoft.com/en-us/securityengineering/sdl)
  - security review と設計段階の gate を考えるときの基礎資料です。
- [OWASP Threat Modeling Cheat Sheet](https://cheatsheetseries.owasp.org/cheatsheets/Threat_Modeling_Cheat_Sheet.html)
  - threat-model workflow を追加するときの根拠です。
- [Google SRE: Release Engineering](https://sre.google/sre-book/release-engineering/)
  - release-readiness と deploy 前後の運用手順の根拠です。
- [Google SRE Workbook: Postmortem Culture](https://sre.google/workbook/postmortem-culture/)
  - postmortem と failure follow-up workflow の根拠です。
- [Microsoft Azure Well-Architected: Safe deployment practices](https://learn.microsoft.com/en-us/azure/well-architected/operational-excellence/safe-deployments)
  - safe deployment と rollback readiness の観点を入れる根拠です。

## Research, Experiment, And Reporting

- [Ten Simple Rules for Reproducible Computational Research](https://doi.org/10.1371/journal.pcbi.1003285)
  - reproducibility review の基礎です。
- [Best Practices for Scientific Computing](https://doi.org/10.1371/journal.pbio.1001745)
  - scientific-computing review の基礎です。
- [Good Enough Practices in Scientific Computing](https://doi.org/10.1371/journal.pcbi.1005510)
  - 軽量な研究運用と実務レベルの checklist の根拠です。
- [Guidance on Reproducibility for Papers Using Computational Tools](https://www.nature.com/articles/d41586-022-00563-z)
  - computational paper の再現性要件を整理するときに使う資料です。
- [Implementing Code Review in the Scientific Workflow](https://doi.org/10.12688/f1000research.27137.2)
  - scientific workflow に code review を組み込む根拠です。
- [Ten Simple Rules for Better Figures](https://doi.org/10.1371/journal.pcbi.1003833)
  - report-review の figure / table 観点の基礎です。
- [Benchmarking in Optimization: Best Practice and Open Issues](https://doi.org/10.48550/arXiv.2007.03488)
  - benchmark fairness と比較条件の根拠です。
- [Benchmarking Crimes: An Emerging Threat in Systems Security](https://doi.org/10.48550/arXiv.1801.02381)
  - benchmarking anti-pattern review の根拠です。
- [Artifact Review and Badging - Current | ACM](https://www.acm.org/publications/policies/artifact-review-and-badging-current)
  - artifact readiness と公開物 completeness の根拠です。
- [The FAIR Guiding Principles for scientific data management and stewardship](https://doi.org/10.1038/sdata.2016.18)
  - FAIR-style data review の根拠です。
- [NeurIPS Paper Checklist](https://nips.cc/public/guides/PaperChecklist)
  - experiment report と claim review の根拠です。
- [REFORMS: Consensus-based Recommendations for Machine-learning-based Science](https://reforms.cs.princeton.edu/)
  - ML-science reporting review の根拠です。

## Related Local Canon

- [references/README.md](/mnt/l/workspace/project_template/references/README.md)
  - reference 置き場の入口です。
- [documents/research-workflow.md](/mnt/l/workspace/project_template/documents/research-workflow.md)
  - 研究・実験改造の正本です。
- [documents/experiment-critical-review.md](/mnt/l/workspace/project_template/documents/experiment-critical-review.md)
  - 批判的レビュー観点の正本です。
- [agents/README.md](/mnt/l/workspace/project_template/agents/README.md)
  - agent canon の入口です。
