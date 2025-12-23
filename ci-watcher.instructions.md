Purpose: Monitor CI runs for the latest commit and triage failures.

When to act:

- If there are local commits to push, push them first to trigger CI.
- Always watch the workflow run for the pushed commit until it completes.

Steps to follow:

1. Push the branch with the latest commits (if needed).
2. Watch the workflow triggered by the latest push (GitHub Actions run).
   - Use `gh run list` / `gh run view <run-id>` to inspect the run and job status.
3. If the run fails, fetch the failed job logs (`gh run view --job <job-id> --log-failed`).
4. Analyze the logs to determine the failure class (configure, build, test, or infra).
5. Propose a fix: explain the root cause in 1–2 lines, then give a minimal code or config change to address it.
6. Implement and push the fix in a small commit; rerun CI and repeat until green.

Additional notes:

- Prefer minimal, reversible changes and include tests where relevant.
- For infra or flaky failures, comment on the run/issue and escalate if needed.
