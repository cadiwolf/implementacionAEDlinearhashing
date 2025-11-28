"use client";

import React, { useEffect, useState, useCallback } from "react";

const API_URL = "http://localhost:8080";

// --- Types ---

type RecordItem = {
  key: string;
  value: string;
};

type Stats = {
  numEntries: number;
  numBuckets: number;
  level: number;
  splitPointer: number;
  loadFactor: number;
  totalSplits: number;
  totalCollisions: number;
};

type BucketInfo = {
  index: number;
  count: number;
  entries: RecordItem[];
};

type StateResponse = {
  stats: Stats;
  buckets?: BucketInfo[];
  records?: RecordItem[];
};

type LogEntry = {
  timestamp: string;
  message: string;
  type: "info" | "success" | "error";
};

// --- Components ---

export default function LinearHashingDashboard() {
  // --- State ---
  const [state, setState] = useState<StateResponse | null>(null);
  const [key, setKey] = useState("");
  const [value, setValue] = useState("");
  const [logs, setLogs] = useState<LogEntry[]>([]);
  const [isPolling, setIsPolling] = useState(true);

  // --- Helpers ---
  const addLog = (message: string, type: LogEntry["type"] = "info") => {
    setLogs((prev) => [
      { timestamp: new Date().toLocaleTimeString(), message, type },
      ...prev.slice(0, 49), // Keep last 50 logs
    ]);
  };

  // --- API Interactions ---

  const fetchState = useCallback(async () => {
    try {
      // Try to fetch full state first
      const res = await fetch(`${API_URL}/state`);
      if (res.ok) {
        const json = await res.json();
        setState(json);
      } else {
        // Fallback: fetch stats and buckets separately if /state is not implemented as a single endpoint
        // This is just a safety measure, assuming the user might have separate endpoints
        const [statsRes, bucketsRes] = await Promise.all([
            fetch(`${API_URL}/stats`),
            fetch(`${API_URL}/buckets`)
        ]);
        
        if (statsRes.ok && bucketsRes.ok) {
            const stats = await statsRes.json();
            const buckets = await bucketsRes.json();
            setState({ stats, buckets });
        }
      }
    } catch {
      // Silent fail on polling to avoid spamming logs, or show a connection status indicator
      // console.error("Error fetching state:", e);
    }
  }, []);

  useEffect(() => {
    fetchState();
    let intervalId: NodeJS.Timeout;
    if (isPolling) {
      intervalId = setInterval(fetchState, 1000);
    }
    return () => clearInterval(intervalId);
  }, [fetchState, isPolling]);

  const handleInsert = async () => {
    if (!key) {
      addLog("Key is required for insertion", "error");
      return;
    }
    try {
      const res = await fetch(`${API_URL}/records`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ key, value }),
      });
      
      if (!res.ok) {
        const txt = await res.text();
        throw new Error(txt);
      }
      
      addLog(`Inserted: ${key} -> ${value}`, "success");
      setKey("");
      setValue("");
      fetchState();
    } catch (e: unknown) {
      const msg = e instanceof Error ? e.message : String(e);
      addLog(`Insert failed: ${msg}`, "error");
    }
  };

  const handleGet = async () => {
    if (!key) {
      addLog("Key is required for search", "error");
      return;
    }
    try {
      const res = await fetch(`${API_URL}/records/${encodeURIComponent(key)}`);
      if (!res.ok) {
        const txt = await res.text();
        throw new Error(txt);
      }
      const json = await res.json();
      // Assuming response is the record object or similar
      const val = json.value || JSON.stringify(json);
      addLog(`Found: ${key} => ${val}`, "success");
      fetchState();
    } catch (e: unknown) {
      const msg = e instanceof Error ? e.message : String(e);
      addLog(`Search failed: ${msg}`, "error");
    }
  };

  const handleDelete = async () => {
    if (!key) {
      addLog("Key is required for deletion", "error");
      return;
    }
    try {
      const res = await fetch(`${API_URL}/records/${encodeURIComponent(key)}`, {
        method: "DELETE",
      });
      if (!res.ok) {
        const txt = await res.text();
        throw new Error(txt);
      }
      addLog(`Deleted: ${key}`, "success");
      setKey("");
      fetchState();
    } catch (e: unknown) {
      const msg = e instanceof Error ? e.message : String(e);
      addLog(`Delete failed: ${msg}`, "error");
    }
  };

  // --- Render Helpers ---

  return (
    <div className="min-h-screen bg-gray-50 p-6 font-sans text-gray-900 dark:bg-gray-900 dark:text-gray-100">
      <div className="mx-auto max-w-7xl space-y-6">
        
        {/* Header */}
        <header className="flex items-center justify-between border-b border-gray-200 pb-4 dark:border-gray-700">
          <div>
            <h1 className="text-3xl font-bold text-blue-600 dark:text-blue-400">Linear Hashing Visualization</h1>
            <p className="text-sm text-gray-500 dark:text-gray-400">
              Backend State: {state ? "Connected" : "Connecting..."}
            </p>
          </div>
          <div className="flex items-center gap-2">
             <label className="flex items-center gap-2 text-sm">
                <input 
                    type="checkbox" 
                    checked={isPolling} 
                    onChange={(e) => setIsPolling(e.target.checked)} 
                    className="rounded border-gray-300"
                />
                Auto-refresh (1s)
             </label>
          </div>
        </header>

        <div className="grid grid-cols-1 gap-6 lg:grid-cols-12">
          
          {/* Left Column: Controls & Stats */}
          <div className="space-y-6 lg:col-span-4">
            
            {/* Control Panel */}
            <section className="rounded-lg bg-white p-5 shadow dark:bg-gray-800">
              <h2 className="mb-4 text-lg font-semibold">Operations</h2>
              <div className="space-y-3">
                <div>
                  <input
                    className="w-full rounded border border-gray-300 p-2 text-sm dark:border-gray-600 dark:bg-gray-700"
                    placeholder="Key"
                    value={key}
                    onChange={(e) => setKey(e.target.value)}
                  />
                </div>
                <div>
                  <input
                    className="w-full rounded border border-gray-300 p-2 text-sm dark:border-gray-600 dark:bg-gray-700"
                    placeholder="Value"
                    value={value}
                    onChange={(e) => setValue(e.target.value)}
                  />
                </div>
                <div className="grid grid-cols-3 gap-2">
                  <button
                    onClick={handleInsert}
                    className="rounded bg-blue-600 px-3 py-2 text-sm font-medium text-white hover:bg-blue-700"
                  >
                    Insert
                  </button>
                  <button
                    onClick={handleGet}
                    className="rounded bg-gray-600 px-3 py-2 text-sm font-medium text-white hover:bg-gray-700"
                  >
                    Search
                  </button>
                  <button
                    onClick={handleDelete}
                    className="rounded bg-red-600 px-3 py-2 text-sm font-medium text-white hover:bg-red-700"
                  >
                    Delete
                  </button>
                </div>
              </div>
            </section>

            {/* Statistics */}
            <section className="rounded-lg bg-white p-5 shadow dark:bg-gray-800">
              <h2 className="mb-4 text-lg font-semibold">Statistics</h2>
              {state ? (
                <div className="grid grid-cols-2 gap-4 text-sm">
                  <div className="rounded bg-gray-100 p-2 dark:bg-gray-700">
                    <span className="block text-xs text-gray-500 dark:text-gray-400">Entries</span>
                    <span className="text-lg font-bold">{state.stats.numEntries}</span>
                  </div>
                  <div className="rounded bg-gray-100 p-2 dark:bg-gray-700">
                    <span className="block text-xs text-gray-500 dark:text-gray-400">Buckets</span>
                    <span className="text-lg font-bold">{state.stats.numBuckets}</span>
                  </div>
                  <div className="rounded bg-gray-100 p-2 dark:bg-gray-700">
                    <span className="block text-xs text-gray-500 dark:text-gray-400">Level</span>
                    <span className="text-lg font-bold">{state.stats.level}</span>
                  </div>
                  <div className="rounded bg-gray-100 p-2 dark:bg-gray-700">
                    <span className="block text-xs text-gray-500 dark:text-gray-400">Split Pointer</span>
                    <span className="text-lg font-bold">{state.stats.splitPointer}</span>
                  </div>
                  <div className="rounded bg-gray-100 p-2 dark:bg-gray-700">
                    <span className="block text-xs text-gray-500 dark:text-gray-400">Load Factor</span>
                    <span className="text-lg font-bold">{state.stats.loadFactor.toFixed(2)}</span>
                  </div>
                  <div className="rounded bg-gray-100 p-2 dark:bg-gray-700">
                    <span className="block text-xs text-gray-500 dark:text-gray-400">Collisions</span>
                    <span className="text-lg font-bold">{state.stats.totalCollisions}</span>
                  </div>
                </div>
              ) : (
                <div className="text-sm text-gray-500">Loading stats...</div>
              )}
            </section>

            {/* Logs */}
            <section className="flex h-64 flex-col rounded-lg bg-white p-5 shadow dark:bg-gray-800">
              <h2 className="mb-2 text-lg font-semibold">Operation Log</h2>
              <div className="flex-1 overflow-y-auto rounded border border-gray-200 bg-gray-50 p-2 text-xs font-mono dark:border-gray-700 dark:bg-gray-900">
                {logs.length === 0 && <span className="text-gray-400">No operations yet.</span>}
                {logs.map((log, i) => (
                  <div key={i} className="mb-1 border-b border-gray-100 pb-1 last:border-0 dark:border-gray-800">
                    <span className="mr-2 text-gray-400">[{log.timestamp}]</span>
                    <span className={
                      log.type === 'error' ? 'text-red-600' : 
                      log.type === 'success' ? 'text-green-600' : 'text-blue-600'
                    }>
                      {log.message}
                    </span>
                  </div>
                ))}
              </div>
            </section>

          </div>

          {/* Right Column: Buckets Visualization */}
          <div className="lg:col-span-8">
            <section className="h-full rounded-lg bg-white p-5 shadow dark:bg-gray-800">
              <div className="mb-4 flex items-center justify-between">
                <h2 className="text-lg font-semibold">Buckets Visualization</h2>
                {state && (
                    <div className="text-xs text-gray-500">
                        Next Split: Bucket {state.stats.splitPointer}
                    </div>
                )}
              </div>
              
              {state?.buckets ? (
                <div className="grid grid-cols-1 gap-4 sm:grid-cols-2 xl:grid-cols-3">
                  {state.buckets.map((bucket) => {
                    const isSplitTarget = state.stats.splitPointer === bucket.index;
                    return (
                        <div 
                            key={bucket.index} 
                            className={`relative flex flex-col rounded-md border-2 p-3 transition-all duration-300 ${
                                isSplitTarget 
                                ? "border-yellow-400 bg-yellow-50 dark:border-yellow-600 dark:bg-yellow-900/20" 
                                : "border-gray-200 bg-gray-50 dark:border-gray-700 dark:bg-gray-900"
                            }`}
                        >
                            <div className="mb-2 flex items-center justify-between border-b border-gray-200 pb-2 dark:border-gray-700">
                                <span className="font-mono text-sm font-bold text-gray-700 dark:text-gray-300">
                                    Bucket {bucket.index}
                                </span>
                                <span className="rounded-full bg-gray-200 px-2 py-0.5 text-xs font-medium text-gray-600 dark:bg-gray-700 dark:text-gray-300">
                                    {bucket.count} items
                                </span>
                            </div>
                            
                            <div className="flex-1 space-y-1">
                                {bucket.entries.length > 0 ? (
                                    bucket.entries.map((entry, idx) => (
                                        <div 
                                            key={`${entry.key}-${idx}`}
                                            className="flex items-center justify-between rounded bg-white px-2 py-1 text-xs shadow-sm dark:bg-gray-800"
                                        >
                                            <span className="font-mono font-semibold text-blue-600 dark:text-blue-400">{entry.key}</span>
                                            <span className="truncate pl-2 text-gray-500">{entry.value}</span>
                                        </div>
                                    ))
                                ) : (
                                    <div className="py-4 text-center text-xs italic text-gray-400">Empty</div>
                                )}
                            </div>
                            
                            {/* Visual indicator for overflow/chaining if needed */}
                            {bucket.entries.length > 5 && (
                                <div className="mt-2 text-center text-xs text-gray-400">
                                    ... {bucket.entries.length - 5} more
                                </div>
                            )}
                        </div>
                    );
                  })}
                </div>
              ) : (
                <div className="flex h-64 items-center justify-center rounded border-2 border-dashed border-gray-200 text-gray-400 dark:border-gray-700">
                  Waiting for bucket data...
                </div>
              )}
            </section>
          </div>

        </div>
      </div>
    </div>
  );
}
